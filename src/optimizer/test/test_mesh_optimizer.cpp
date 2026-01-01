// Sriramajayam

#include <vm_mesh_optimizer.h>
#include <vm_face_qualities.h>
#include <vm_io.h>
#include <random>

// test face quality evaluation
void test_evaluate_face_quality(const pmp::SurfaceMesh&, const vm::QualityEvaluator&);

// test vertex quality evaluation
void test_evaluate_vertex_quality(const pmp::SurfaceMesh&, const vm::QualityEvaluator&);

// test relax() overload 1
void test_relax_1(const pmp::SurfaceMesh&, const vm::QualityEvaluator&);

// test relax() overload 2
void test_relax_2(const pmp::SurfaceMesh&, const vm::QualityEvaluator&);

// test relax() overload 3
void test_relax_3(const pmp::SurfaceMesh&, const vm::QualityEvaluator&);

// test agglomerate() overload 1
void test_agglomerate_1(const pmp::SurfaceMesh&, const vm::QualityEvaluator&);

// test agglomerate() overload 2
void test_agglomerate_2(const pmp::SurfaceMesh&, const vm::QualityEvaluator&);

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

  // overload 2 for relaxation
  test_relax_2(mesh, QE);

  // overload 3 for relaxation
  test_relax_3(mesh, QE);

  // test agglomerate() overload 1
  test_agglomerate_1(mesh, QE);
 
  // test agglomerate() overload 2
  //test_agglomerate_2(mesh, QE);
  
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
  

// ------ test that vector u < vector v ----- //
// assumes that u and v are sorted in ascending order and have identical length
// u > v if there exist n such that
// u[i] >= v[i] for i<=n
bool is_greater(const std::vector<double>& u, const std::vector<double>& v)
{
  if(u.size()!=v.size())
    {
      std::cerr << "\nis_greater: cannot compare vectors of different sizes\n";
      std::exit(EXIT_FAILURE);
    }
  const int nsize = static_cast<int>(u.size());
  const double EPS = 1.e-6;

  // find the first index n for which u[i] neq v[i]
  int n = nsize;
  for(int i=0; i<nsize; ++i)
    if(std::abs(u[i]-v[i])>EPS)
      {
	n = i;
	break;
      }
  
  // expect u[i]>=v[i] until index n
  for(int i=0; i<n; ++i)
    if(u[i]<v[i]-EPS)
      return false;

  return true;
}


// --------- test relax() overload 2 -------//
void test_relax_2(const pmp::SurfaceMesh& in_mesh,
		  const vm::QualityEvaluator &QE)
{
  // generate a random subset of relaxable vertices
  // relax them and check consistency of the vector-valued mesh quality
  vm::MeshOptimizer opt(in_mesh);
  auto& mesh = opt.get_mesh();
  auto v_circulator = mesh.vertices();

  // compute face/vertex qualities before relaxation
  const std::string f_tag_pre = "geom_face_quality_pre";
  const std::string v_tag_pre = "geom_vertex_quality_pre";
  opt.evaluate_face_qualities(QE, f_tag_pre);
  opt.evaluate_vertex_qualities(f_tag_pre, v_tag_pre);
  
  std::set<pmp::Vertex> subset{};
  std::random_device rd;
  std::mt19937 gen(rd());
  std::bernoulli_distribution d(0.5);

  for(auto v:v_circulator)
    if(!mesh.is_boundary(v))
      if(d(gen))
	subset.insert(v);

  // relax vertices
  opt.relax(subset, QE, 4); // 4 samples

  // compute face/vertex qualities after relaxation
  const std::string f_tag_post = "geom_face_quality_post";
  const std::string v_tag_post = "geom_vertex_quality_post";
  opt.evaluate_face_qualities(QE, f_tag_post);
  opt.evaluate_vertex_qualities(f_tag_post, v_tag_post);

  // face quality vectors
  std::vector<double> q_face_pre{}, q_face_post{};
  auto prop_q_face_pre  = mesh.get_face_property<double>(f_tag_pre); 
  auto prop_q_face_post = mesh.get_face_property<double>(f_tag_post);
  auto f_circulator = mesh.faces();
  for(auto f:f_circulator)
    {
      q_face_pre.push_back(prop_q_face_pre[f]);
      q_face_post.push_back(prop_q_face_post[f]);
    } 
  std::sort(q_face_pre.begin(), q_face_pre.end());
  std::sort(q_face_post.begin(), q_face_post.end());

  // vertex quality vectors
  std::vector<double> q_vertex_pre{}, q_vertex_post{};
  auto prop_q_vertex_pre  = mesh.get_vertex_property<double>(v_tag_pre);
  auto prop_q_vertex_post = mesh.get_vertex_property<double>(v_tag_post);
  for(auto v:v_circulator)
    {
      q_vertex_pre.push_back(prop_q_vertex_pre[v]);
      q_vertex_post.push_back(prop_q_vertex_post[v]);
    }
  std::sort(q_vertex_pre.begin(),  q_vertex_pre.end());
  std::sort(q_vertex_post.begin(), q_vertex_post.end());

  // check ordering of face quality vectors
  if(!is_greater(q_face_post, q_face_pre))
    {
      std::cerr << "\ntest_relax_2: pre face quality > post face quality\n";
      std::exit(EXIT_FAILURE);
    }

  // check ordering of vertex quality vectors
  if(!is_greater(q_vertex_post, q_vertex_pre))
    {
      std::cerr << "\ntest_relax_2: pre vertex quality > post vertex quality\n";
      std::exit(EXIT_FAILURE);
    }

}


// --------- test relax() overload 3 -------//
void test_relax_3(const pmp::SurfaceMesh& in_mesh,
		  const vm::QualityEvaluator &QE)
{
  // optimizer
  vm::MeshOptimizer opt(in_mesh);
  auto& mesh = opt.get_mesh();
  //vm::write_vtk(mesh, "in.vtk");
  
  // random lower bound for quality
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> d(0.25, 0.8);
  const double qmin = d(gen);

  // compute face/vertex qualities before relaxation
  const std::string f_tag_pre = "geom_face_quality_pre";
  const std::string v_tag_pre = "geom_vertex_quality_pre";
  opt.evaluate_face_qualities(QE, f_tag_pre);
  opt.evaluate_vertex_qualities(f_tag_pre, v_tag_pre);

  // relax
  opt.relax(QE, qmin, 4);
  //vm::write_vtk(mesh, "out.vtk");

  // compute face/vertex qualities after relaxation
  const std::string f_tag_post = "geom_face_quality_post";
  const std::string v_tag_post = "geom_vertex_quality_post";
  opt.evaluate_face_qualities(QE, f_tag_post);
  opt.evaluate_vertex_qualities(f_tag_post, v_tag_post);

  // face quality vectors
  std::vector<double> q_face_pre{}, q_face_post{};
  auto prop_q_face_pre  = mesh.get_face_property<double>(f_tag_pre); 
  auto prop_q_face_post = mesh.get_face_property<double>(f_tag_post);
  auto f_circulator = mesh.faces();
  for(auto f:f_circulator)
    {
      q_face_pre.push_back(prop_q_face_pre[f]);
      q_face_post.push_back(prop_q_face_post[f]);
    } 
  std::sort(q_face_pre.begin(), q_face_pre.end());
  std::sort(q_face_post.begin(), q_face_post.end());

  // vertex quality vectors
  std::vector<double> q_vertex_pre{}, q_vertex_post{};
  auto prop_q_vertex_pre  = mesh.get_vertex_property<double>(v_tag_pre);
  auto prop_q_vertex_post = mesh.get_vertex_property<double>(v_tag_post);
  auto v_circulator = mesh.vertices();
  for(auto v:v_circulator)
    {
      q_vertex_pre.push_back(prop_q_vertex_pre[v]);
      q_vertex_post.push_back(prop_q_vertex_post[v]);
    }
  std::sort(q_vertex_pre.begin(),  q_vertex_pre.end());
  std::sort(q_vertex_post.begin(), q_vertex_post.end());

  // check ordering of face quality vectors
  if(!is_greater(q_face_post, q_face_pre))
    {
      std::cerr << "\ntest_relax_3: pre face quality > post face quality\n";
      std::exit(EXIT_FAILURE);
    }

  // check ordering of vertex quality vectors
  if(!is_greater(q_vertex_post, q_vertex_pre))
    {
      std::cerr << "\ntest_relax_3: pre vertex quality > post vertex quality\n";
      std::exit(EXIT_FAILURE);
    }

}


// -------- test agglomerate() overload 1 ------------ //
void test_agglomerate_1(const pmp::SurfaceMesh &mesh,
			const vm::QualityEvaluator &QE)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> d1(0.01, 0.1);
  const double qmin = d1(gen);
  std::uniform_real_distribution<double> d2(1.01, 1.2);
  const double qfactor = d2(gen);

  const int nfaces = mesh.n_faces();
  
  // try agglomerating each face
  auto f_circulator = mesh.faces();
  for(auto f:f_circulator)
    {
      // quality of this face
      double curr_quality = QE(f, mesh);

      // neighbors of this face
      auto h_edges = mesh.halfedges();
      std::set<int> nbs{};
      for(auto h:h_edges)
	if(!mesh.is_boundary(h))
	  nbs.insert(mesh.face(mesh.opposite_halfedge(h)).idx());
      
      // optimizer
      vm::MeshOptimizer opt(mesh);

      // try agglomerating this face
      auto result = opt.agglomerate(f, QE, qmin, qfactor);

      const bool success = std::get<bool>(result);
      const int new_idx = std::get<pmp::Face>(result).idx();
      const double new_quality = std::get<double>(result);
      
      // consistency checks in case of success
      if(success)
	{
	  // quality improvement
	  if( !(new_quality>=qfactor*curr_quality && new_quality>qmin) )
	    {
	      std::cerr << "\ntest_agglomerate_1: inconsistency in merged face quality\n";
	      std::exit(EXIT_FAILURE);
	    }

	  // number of faces
	  if(opt.get_mesh().n_faces()!=nfaces-1)
	    {
	      std::cerr << "\ntest_agglomerate_1: inconsistency in number of faces after successful merge\n";
	      std::exit(EXIT_FAILURE);
	    }

	  // new face should equal this face or one of the neighbors
	  if( new_idx!=f.idx() && nbs.find(new_idx)==nbs.end() )
	    {
	      std::cerr << "\ntest_agglomerate_1: inconsistency in agglomerated face after successful merge\n";
	      std::exit(EXIT_FAILURE);
	    }
	}
      // consistency checks in case of failure
      else 
	{
	   // number of faces
	  if(opt.get_mesh().n_faces()!=nfaces)
	    {
	      std::cerr << "\ntest_agglomerate_1: inconsistency in number of faces after unsuccessful merge\n";
	      std::exit(EXIT_FAILURE);
	    }

	  // new face should equal old one
	  if( new_idx!=f.idx() )
	    {
	      std::cerr << "\ntest_agglomerate_1: inconsistency in agglomerated face after unsuccessful merge\n";
	      std::exit(EXIT_FAILURE);
	    }
	}
    }
}
