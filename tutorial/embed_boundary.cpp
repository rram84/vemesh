// Sriramajayam

/** \file embed_boundary.cpp
 * \brief Tutorial-style example embedding a circular boundary in a triangle mesh and subsequent mesh improvement with vemesh
 * \ingroup tutorial
 */

#include <vm_mesh_optimizer.h>
#include <vm_face_qualities.h>
#include <vm_io.h>
#include <vm_tutorial_mesh_slicer.h>
#include <filesystem>
#include <CLI/CLI.hpp>

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
  // ---- non-conforming mesh and domain inputs --- 

  // input triangle mesh over [-1,1] x [-1,1] 
  const std::string meshfile = "sample_data/tri/bbbb-3.off";
  pmp::SurfaceMesh tri_mesh = vm::read_off(meshfile);
  
  // center and radius of circular domain
  const double circ_center[] = {0.,0.};
  const double circ_radius = 1./std::sqrt(3.);

  // level set function for circular boundary
  vm::tutorial::LevelSetFn ls_circ =
    [circ_center, circ_radius](const double* X) {
    double Y[] = {X[0]-circ_center[0], X[1]-circ_center[1]};
    return std::sqrt(Y[0]*Y[0]+Y[1]*Y[1])-circ_radius;
  };

  // --- output directory ---
  const std::string outdir = "output";

  // clean up vtk files 
  namespace fs = std::filesystem;
  const fs::path outpath = fs::path(outdir);
  fs::create_directories(outpath);
  for(const auto& e : fs::directory_iterator(outpath)) {
    if(e.is_regular_file() && e.path().extension() == ".vtk")
      fs::remove(e);
  }

  // ---- embed the circular boundary in the triangle mesh ---

  // perturb mesh nodes away from the zero level set
  const double phi_tol = 1.e-5; 
  const double pert_dist = 10.*phi_tol;
  vm::tutorial::adjust_mesh_nodes(tri_mesh, phi_tol, pert_dist, ls_circ);

  // embed the circular boundary in the perturbed mesh
  pmp::SurfaceMesh embedded_mesh = vm::tutorial::clip_mesh(tri_mesh, phi_tol, ls_circ);

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
    std::cout << "\nrealxed " << num_relaxed << " vertices " << std::flush;

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
