// Sriramajayam

/** \file embed_interface.cpp
 * \brief Tutorial-style example embedding an interface (polygonal chain) in a structured quad mesh and subsequent mesh improvement with vemesh
 * \ingroup tutorial
 */

#include <vm_mesh_optimizer.h>
#include <vm_face_qualities.h>
#include <vm_io.h>
#include <vm_tutorial_rectangle_mesh.h>
#include <vm_tutorial_mesh_slicer.h>
#include <vm_tutorial_polygonSDF.h>
#include <filesystem>
#include <fstream>

// identify candidate faces for agglomeration
std::set<pmp::Face> identify_candidate_faces(const pmp::SurfaceMesh&,
					     const vm::QualityEvaluator&,
					     const double);

// identify candidate vertices for relaxation
std::set<pmp::Vertex> identify_candidate_vertices(const pmp::SurfaceMesh&,
						  const vm::QualityEvaluator&,
						  const double);

int main()
{
  // --- define the interface using sampling provided in file ---
  const std::string filename_interface_vertices = "sample_data/shapes/85909.dat";

  // signed distance function calculator to the interface
  const vm::tutorial::PolygonSDF interface_sdf(filename_interface_vertices);

  // signed-distance level set to the polygon
  vm::tutorial::LevelSetFn sdfunc = [&interface_sdf](const double* X) { return interface_sdf(X); };
  
  // --- generate a structured quad mesh (not conforming to the interface) ---
  const std::array<double,2> left_cnr{-1.5,-1.5}; // bottom left corner
  const int nx = 25; // #nodes along x
  const int ny = 25; // #nodes along y
  const double hx = 3.0/static_cast<double>(nx-1); // grid size along x
  const double hy = 3.0/static_cast<double>(ny-1); // grid size along y
  auto rect_mesh = vm::tutorial::create_rectangle_mesh(left_cnr, hx, nx, hy, ny);

  // --- output directory ---
  const std::string outdir = "output";

  // Remove existing VTK files
  namespace fs = std::filesystem;
  const fs::path outpath = fs::path(outdir);
  fs::create_directories(outpath);
  for(const auto& e : fs::directory_iterator(outpath)) {
    if(e.is_regular_file() && e.path().extension() == ".vtk")
      fs::remove(e);
  }

  // ---- embed the interface in the background mesh ---

  // perturb mesh nodes away from the zero level set
  const double phi_tol = 1.e-5; 
  const double pert_dist = 10.*phi_tol;
  vm::tutorial::adjust_mesh_nodes(rect_mesh, phi_tol, pert_dist, sdfunc);

  // embed the interface in the perturbed mesh
  pmp::SurfaceMesh embedded_mesh = vm::tutorial::embed_interface(rect_mesh, phi_tol, sdfunc);

  // --- algorithmic parameters for agglomeration ---
  
  // number of relaxation+agglomeration iterations to perform
  const int num_iters = 5;

  // lower bound for element quality
  const double qepsilon = 0.3;

  // improvement factor to accept an agglomerated element
  const double qfactor = 1.2;

  // number of samples for relaxation
  const int num_samples = 5;
  
  // ---- face quality metric --- 
  const auto face_quality_metric = vm::quality::vem_stability_ratio;

  // ----  quality evaluator ---
  vm::QualityEvaluator QE(face_quality_metric);

  // --- mesh optimizer ---
  vm::MeshOptimizer optimizer(embedded_mesh);
  auto& mesh = optimizer.get_mesh();    // this is the mesh mutated by the optimizer

  // --- evaluate and save initial mesh quality ---
  optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
  vm::write_vtk(mesh, outdir+"/embedded_mesh.vtk");
  vm::write_face_quality_vector(mesh, outdir+"/qvec-input.dat");

  // --- iteratively optimizer ---
  for(int iter=1; iter<=num_iters; ++iter) {
    
    std::cout << "\n\n Iteration " << iter <<": " << std::flush;

    // relax 
    auto relax_vertices = identify_candidate_vertices(mesh, QE, qepsilon); // candidate vertices
    int num_relaxed = optimizer.relax(relax_vertices, QE, num_samples);
    std::cout << "\nrelaxed " << num_relaxed << " vertices " << std::flush;

    // agglomerate
    auto agg_faces = identify_candidate_faces(mesh, QE, qepsilon); // candidate faces
    int num_agg = optimizer.agglomerate(agg_faces, QE, qfactor);
    std::cout << "\nagglomerated " << num_agg << " faces " << std::flush;

    // evaluate mesh qualities and save file
    optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
    vm::write_vtk(mesh, outdir+"/mesh-iter-"+std::to_string(iter)+".vtk");
    vm::write_face_quality_vector(mesh, outdir+"/qvec-iter-"+std::to_string(iter)+".dat");
  }

  // --- save the final mesh ---
  vm::write_vtk(mesh, outdir+"/output.vtk");
  vm::write_face_quality_vector(mesh, outdir+"/qvec-output.dat");
}


// identify candidate faces for agglomeration
std::set<pmp::Face> identify_candidate_faces(const pmp::SurfaceMesh &mesh,
					     const vm::QualityEvaluator &QE,
					     const double qepsilon)
{
  std::set<pmp::Face> agg_faces{};

  // candidate faces for agglomeration
  // (i) has to have least one node on the zero level set, i.e., interface_id = 1
  // (ii) quality < threshold
  auto interface_ids = mesh.get_vertex_property<int>("interface_id");
  auto faces = mesh.faces();
  for(auto f:faces)
    {
      auto vertices = mesh.vertices(f);
      for(auto v:vertices)
	if(interface_ids[v]==1)
	  {
	    
	    double qval = QE(f, mesh); // quality of this face
	    if(qval<qepsilon)
	      // this is a candidate face for agglomeration
	      agg_faces.insert(f);
	    break;
	  }
    }

  return agg_faces;
}


// identify candidate vertices for relaxation
std::set<pmp::Vertex> identify_candidate_vertices(const pmp::SurfaceMesh &mesh,
						  const vm::QualityEvaluator &QE,
						  const double qepsilon)
{
  std::set<pmp::Vertex> relax_vertices{};
  auto interface_ids = mesh.get_vertex_property<int>("interface_id");
  
  // (i)   should not lie on the zero level set
  // (ii)  should be connected to a node on the zero level set by an edge in the mesh
  // (iii) quality < threshold
  auto vertices = mesh.vertices();
  for(auto v:vertices)
    if(interface_ids[v]==1) // this is a boundary node
      {
	auto vertex_nbs = mesh.vertices(v); // its 1-ring
	for(auto w:vertex_nbs)
	  if(interface_ids[w]==-1)          // this is an interior node
	    if(relax_vertices.count(w)==0)
	      {
		double qval = QE(w, mesh);    // quality of this vertex
		if(qval<qepsilon)
		  relax_vertices.insert(w);   // this is a candidate vertex for relaxation
	      }
      }
  
  return relax_vertices;
}
