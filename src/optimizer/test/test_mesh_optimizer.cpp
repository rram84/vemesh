// Sriramajayam

#include <vm_mesh_optimizer.h>
#include <vm_face_qualities.h>
#include <vm_io.h>

// test relax() overload 1
void test_relax_1(const pmp::SurfaceMesh&, const vm::QualityEvaluator&);

int main()
{
  // read a polygonal mesh
  auto mesh = vm::read_off("sample_meshes/sorgente_mesh1_40.off");

  // geometric quality
  vm::QualityEvaluator QE(vm::quality::geom_shape);

  // overload 1 for relaxation
  test_relax_1(mesh, QE);
}



// test relax() overload 1
void test_relax_1(const pmp::SurfaceMesh& in_mesh,
		  const vm::QualityEvaluator& QE)
{
  vm::MeshOptimizer opt(in_mesh);
  auto& mesh = opt.get_mesh();
  auto v_circulator = mesh.vertices();
  for(auto v:v_circulator)
    if(!mesh.is_boundary(v))
      {
	double curr_quality = QE(v, mesh);
	pmp::Point X = mesh.position(v);
	
	// relax
	auto result = opt.relax(v, QE, 4); // nsamples = 4

	// check for improvement and perturbation
	if(std::get<bool>(result)==true)
	  {
	    // new vertex coordinates and quality
	    pmp::Point Y = mesh.position(v);
	    double new_quality = std::get<double>(result);
	    
	    if(new_quality<=curr_quality)
	      {
		std::cerr <<"\ntest_relax_1: inconsistent quality after vertex update\n";
		std::exit(EXIT_FAILURE);
	      }
	    if(pmp::norm(X-Y)<1.e-8)
	      {
		std::cerr <<"\ntest_relax_1: unexpected vertex coordinates after update\n";
		std::exit(EXIT_FAILURE);
	      }
	  }
      }
}
  
