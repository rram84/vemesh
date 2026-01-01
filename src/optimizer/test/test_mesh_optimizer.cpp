// Sriramajayam

#include <vm_mesh_optimizer.h>
#include <vm_face_qualities.h>
#include <vm_io.h>

// test face quality evaluation
void test_evaluate_face_quality(const pmp::SurfaceMesh&, const vm::QualityEvaluator&);

// test vertex quality evaluation
void test_evaluate_vertex_quality(const pmp::SurfaceMesh&, const vm::QualityEvaluator&);

// test relax() overload 1
void test_relax_1(const pmp::SurfaceMesh&, const vm::QualityEvaluator&);

// test relax() overload 2
void test_relax_2(const pmp::SurfaceMesh&, const vm::QualityEvaluator&);

int main()
{
  // read a polygonal mesh
  auto mesh = vm::read_off("sample_meshes/sorgente_mesh1_40.off");

  // geometric quality
  vm::QualityEvaluator QE(vm::quality::geom_shape);

  // test face quality evaluation
  test_evaluate_face_quality(mesh, QE);

  // test vertex quality evaluation
  test_evaluate_vertex_quality(mesh, QE);

  // overload 1 for relaxation
  test_relax_1(mesh, QE);

}

// ------- test face quality evaluation -------- //
void test_evaluate_face_quality(const pmp::SurfaceMesh& in_mesh,
				const vm::QualityEvaluator& QE)
{
  // evaluate face qualities using the class
  vm::MeshOptimizer opt(in_mesh);
  opt.evaluate_face_qualities(QE, "geom_face_qualities");
  const auto& mesh = opt.get_mesh();
  const auto face_qualities = mesh.get_face_property<double>("geom_face_qualities");
  if(!mesh.has_face_property("geom_face_qualities"))
    {
      std::cerr << "\ntest_evaluate_face_quality: face property for quality not created.\n";
      std::exit(EXIT_FAILURE);
    }
  
  // evaluate face qualities directly and compare
  auto f_circulator = mesh.faces();
  for(auto f:f_circulator)
    {
      auto vertices = mesh.vertices(f);
      std::vector<pmp::Point> coords{};
      for(auto v:vertices)
	coords.push_back(mesh.position(v));

      double qval_1 = QE(coords);
      double qval_2 = face_qualities[f];
      if(std::abs(qval_1-qval_2)>1.e-6)
	{
	  std::cerr << "\ntest_evaluate_face_quality: inconsistency in face quality evaluation\n";
	  std::exit(EXIT_FAILURE);
	}
    }
}

// ------ test vertex quality evaluation ------ //
void test_evaluate_vertex_quality(const pmp::SurfaceMesh &in_mesh,
				  const vm::QualityEvaluator &QE)
{
  const std::string face_quality_tag = "geom_face_quality";
  const std::string vertex_quality_tag = "geom_vertex_quality";
  
  // evaluate face qualities using the class
  vm::MeshOptimizer opt(in_mesh);
  const auto& mesh = opt.get_mesh();
  opt.evaluate_face_qualities(QE, face_quality_tag);

  // Evaluate vertex qualities
  opt.evaluate_vertex_qualities(face_quality_tag, vertex_quality_tag);
  if(!mesh.has_vertex_property(vertex_quality_tag))
    {
      std::cerr << "\ntest_evaluate_vertex_quality: vertex property for quality not created.\n";
      std::exit(EXIT_FAILURE);
    }

  // access vertex qualities from the mesh
  const auto vertex_qualities = mesh.get_vertex_property<double>(vertex_quality_tag);
  
  // evaluate vertex qualities directly and compare
  auto v_circulator = mesh.vertices();
  for(auto v:v_circulator)
    {
      double qval_1 = QE(v, mesh);
      double qval_2 = vertex_qualities[v];
      if(std::abs(qval_1-qval_2)>1.e-6)
	{
	  std::cerr << "\ntest_evaluate_vertex_quality: inconsistency in vertex quality evaluation\n";
	  std::exit(EXIT_FAILURE);
	}
    }
}	



// --------- test relax() overload 1 ---------- //
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
  


// --------- test relax() overload 2 -------//
void test_relax_2(const pmp::SurfaceMesh& in_mesh, const vm::QualityEvaluator &QE)
{
}
