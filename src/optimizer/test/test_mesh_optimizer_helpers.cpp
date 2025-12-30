// Sriramajayam

#include <vm_mesh_optimizer.h>
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

int main()
{
  // read a sample mesh
  auto mesh = vm::read_off("sample_meshes/random_triangles.OFF");
  vm::write_vtk(mesh, "tri.vtk");
  
  // testable mesh optimizer
  TestMeshOptimizer opt(mesh);

  // test agglomerability
  test_agglomerability(opt);
  
  // test face merging
  test_merge_faces(mesh);

  // test feasible vertex positions
  //test_feasible_vertex_positions(opt);
  
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
      
