// Sriramajayam

/** \file test_mesh_optimizer_helpers.cpp
 * \brief Unit tests for protected helper methods of the class vm::MeshOptimizer
 * \author Ramsharan Rangarajan
 */

#include <vm_mesh_optimizer.h>
#include <vm_mesh_inspection.h>
#include <vm_face_qualities.h>
#include <vm_io.h>

// Class for testing
class TestMeshOptimizer: public vm::MeshOptimizer
{
public:
  TestMeshOptimizer(const pmp::SurfaceMesh& mesh)
    :MeshOptimizer(mesh) {}

  // Helper method to check agglomerability
  using vm::MeshOptimizer::is_agglomerable;
  
  // Helper method to identify the optimal agglomerable neighbor for merging a given face
  using vm::MeshOptimizer::find_halfedge_for_face_merge;
    
  // Helper method to execute a merge faces incident at a given halfedge
  using vm::MeshOptimizer::merge_neighbors;
    
  // Helper method to generate feasible positions to perturb a given vertex to
  using vm::MeshOptimizer::compute_feasible_vertex_positions;

  // Helper method to compute new location for a vertex
  using vm::MeshOptimizer::compute_improved_vertex_position;
  
};

// test merge face
void test_merge_faces(const pmp::SurfaceMesh&);

// test agglomeration criterion
void test_agglomerability(const TestMeshOptimizer&);

// test feasible vertex positions
void test_feasible_vertex_positions(const TestMeshOptimizer&);

// test optimal vertex location
void test_improved_vertex_position(const pmp::SurfaceMesh&, const vm::QualityEvaluator&);

// test optimal halfedge for agglomeration
void test_optimal_halfedge(const TestMeshOptimizer&, const vm::QualityEvaluator&);

int main()
{
  // read a sample mesh
  auto mesh = vm::read_off("sample_meshes/sorgente_mesh1_40.off"); // random_triangles.OFF
  vm::write_vtk(mesh, "tri.vtk");
  
  // testable mesh optimizer
  TestMeshOptimizer opt(mesh);

  // test agglomerability
  test_agglomerability(opt);
  
  // test face merging
  test_merge_faces(mesh);

  // test feasible vertex positions
  test_feasible_vertex_positions(opt);

  // geometric quality
  vm::QualityEvaluator QE(vm::quality::geom_shape);

  // test optimal vertex relaxation
  test_improved_vertex_position(mesh, QE);

  // test optimal halfedge for agglomeration
  test_optimal_halfedge(opt, QE);
}


// test merge face
void test_merge_faces(const pmp::SurfaceMesh& in_mesh)
{
  auto h_edges = in_mesh.halfedges();
  for(auto h:h_edges)
    {
      TestMeshOptimizer opt(in_mesh);
      if(opt.is_agglomerable(h))
	{
	  auto f0 = in_mesh.face(h);
	  auto f1 = in_mesh.face(in_mesh.opposite_halfedge(h));
	  int dom_id = in_mesh.get_face_property<int>("domain_id")[f0];
	  
	  // merge
	  auto new_f = opt.merge_neighbors(h);

	  // mutated mesh
	  auto mesh = opt.get_mesh();
	  
	  // new_f = f0 or f1
	  if( !mesh.is_valid(new_f) ||
	      !(new_f.idx()==f0.idx() || new_f.idx()==f1.idx()) ||
	      !(mesh.is_deleted(f0) || mesh.is_deleted(f1)) ||
	      !(mesh.is_valid(f0) || mesh.is_valid(f1)) )
	    {
	      std::cerr << "\ntest_merge_faces: unexpected result of face merge\n";
	      std::exit(EXIT_FAILURE);
	    }

	  // new face should have the right domain id
	  if(mesh.get_face_property<int>("domain_id")[new_f]!=dom_id)
	    {
	      std::cerr << "\ntest_merge_faces: merged face has incorrect domain id\n";
	      std::exit(EXIT_FAILURE);
	    }

	  // vertex, edge and face count
	  if(mesh.n_vertices()!=in_mesh.n_vertices() ||
	     mesh.n_edges()!=in_mesh.n_edges()-1 ||
	     mesh.n_faces()!=in_mesh.n_faces()-1)
	    {
	      std::cerr << "\ntest_merge_faces: unexpected face/edge/vertex count\n";
	      std::exit(EXIT_FAILURE);
	    }
	}
    }   
}


// test agglomeration criterion
void test_agglomerability(const TestMeshOptimizer& opt)
{
  auto mesh = opt.get_mesh();
  auto h_edges = mesh.halfedges();
  auto domain_id = mesh.get_face_property<int>("domain_id");
  
  for(auto h:h_edges)
    if(!opt.is_agglomerable(h))
      {
	// on boundary
	if(mesh.is_boundary(mesh.edge(h)))
	  continue;

	// isolated vertex
	if(mesh.valence(mesh.from_vertex(h))<=2)
	  continue;
	if(mesh.valence(mesh.to_vertex(h))<=2)
	  continue;

	// domain_id
	auto f = mesh.face(h);
	auto nb_f = mesh.face(mesh.opposite_halfedge(h));
	if(domain_id[f]!=domain_id[nb_f])
	  continue;

	// unexpected reason
	std::cerr << "\ntest_agglomerability: unexpected reason\n";
      }
}
      
// test feasible vertex positions
void test_feasible_vertex_positions(const TestMeshOptimizer& opt)
{
  auto mesh = opt.get_mesh();
  auto v_circulator = mesh.vertices();
  for(auto v:v_circulator)
    if(!mesh.is_boundary(v))
      {
	// get feasible locations
	auto points = opt.compute_feasible_vertex_positions(v, 4);

	// mutable mesh
	pmp::SurfaceMesh tmp = mesh;
	
	// check that moving v -> new location retains mesh sanctity
	auto f_circulator = tmp.faces(v);
	std::set<pmp::Face> faces{};
	for(auto f:f_circulator)
	  faces.insert(f);
	
	// mutate for each feasible location
	for(auto pt:points)
	  {
	    pmp::Point X(pt.first, pt.second, 0.);
	    tmp.position(v) = X;

	    // inspect modified faces
	    std::vector<vm::InspectionError> errors{}; 
	    bool flag = vm::inspect_mesh(tmp, vm::MeshInspection::Adjacency, errors);
	    if(flag==false)
	      {
		std::cerr << "\ntest_feasible_vertex_positions: unexpected failure with feasible point \nErrors: \n";
		for(auto &e:errors) std::cerr << e << "\n";
		std::exit(EXIT_FAILURE);
	      }
	  }
      }
}


// test optimal vertex location
void test_improved_vertex_position(const pmp::SurfaceMesh &in_mesh, const vm::QualityEvaluator &QE)
{
  TestMeshOptimizer opt(in_mesh);
  auto mesh = opt.get_mesh();
  auto v_circulator = mesh.vertices();
  for(auto v:v_circulator)
    if(!mesh.is_boundary(v))
      {
	auto result = opt.compute_improved_vertex_position(v, 4, QE);
	if(std::get<bool>(result)==true)
	  {
	    // quality should be improved
	    double curr_quality = QE(v,mesh);
	    if(std::get<double>(result)<curr_quality)
	      {
		std::cerr << "\ntest_improved_vertex_position: quality did not improve\n";
		std::exit(EXIT_FAILURE);
	      }

	    // new location should be feasible
	    pmp::SurfaceMesh tmp = mesh;
	    tmp.position(v) = std::get<pmp::Point>(result);
	    std::vector<vm::InspectionError> errors{}; 
	    bool flag = vm::inspect_mesh(tmp, vm::MeshInspection::Adjacency, errors);
	    if(flag==false)
	      {
		std::cerr << "\ntest_improved_vertex_position: optimal pointis not feasible\n";
		for(auto &e:errors) std::cerr << e << "\n";
		std::exit(EXIT_FAILURE);
	      }

	    // check quality
	    double check_quality = QE(v, tmp);
	    if(std::abs(check_quality-std::get<double>(result))>1.e-4)
	      {
		std::cerr << "\ntest_improved_vertex_position: inconsistency in optimal quality\n";
		for(auto &e:errors) std::cerr << e << "\n";
		std::exit(EXIT_FAILURE);
	      }
	  }
      }
}


// test optimal halfedge for agglomeration
void test_optimal_halfedge(const TestMeshOptimizer &opt, const vm::QualityEvaluator &QE)
{
  auto mesh = opt.get_mesh();
  auto f_circulator = mesh.faces();
  for(auto face:f_circulator)
    {
      // optimal halfedge for merge
      auto result = opt.find_halfedge_for_face_merge(face, QE);

      // verify result with feasible merges
      auto halfedges = mesh.halfedges(face);
      bool success = false;
      double opt_quality = -1.;
      pmp::Halfedge opt_h;
      for(auto h:halfedges)
	if(opt.is_agglomerable(h))
	  {
	    TestMeshOptimizer tmp(mesh);
	    auto new_face = tmp.merge_neighbors(h);
	    double new_quality = QE(new_face, tmp.get_mesh());
	    if(new_quality>opt_quality)
	      {
		opt_quality = new_quality;
		opt_h = h;
		success = true;
	      }
	  }

      // validate
      if(std::get<bool>(result)!=success)
	{
	  std::cerr << "\ntest_optimal_halfedge: discrepancy in feasibility.\n";
	  std::exit(EXIT_FAILURE);
	}
      if(std::get<pmp::Halfedge>(result).idx()!=opt_h.idx())
	{
	  std::cerr << "\ntest_optimal_halfedge: discrepancy in optimal halfedge\n";
	  std::exit(EXIT_FAILURE);
	}
      if(std::abs(std::get<double>(result)-opt_quality)>1.e-4)
	{
	  std::cerr << "\ntest_optimal_halfedge: discrepancy in optimal quality\n";
	  std::exit(EXIT_FAILURE);
	}
    }
}

  
