// Sriramajayam

/** \file test_environment.cpp
 * \brief Unit tests for utility routines vm::get_vertex_environment and vm::get_environment_polygon defined in vm_utils.h
 * \ingroup tests
 * \author Ramsharan Rangarajan
 */

#include <vm_utils.h>
#include <vm_io.h>

// test the get_environment_vertices function
void test_environment_vertices(const pmp::SurfaceMesh&);

// test the environment polygon
void test_environment_polygon(const pmp::SurfaceMesh&);

// test erase non manifold edges 
void test_erase_nonmanifold_edges();
  
int main()
{
  // read a sample mesh
  auto mesh = vm::read_off("sample_meshes/sorgente_mesh1_40.off");  //random_triangles.OFF";
  //vm::write_vtk(mesh, "sorgente.vtk");
  
  // test environment vertices
  test_environment_vertices(mesh);

  // test the environment polygon
  test_environment_polygon(mesh);

  // test erase non manifold edges
  test_erase_nonmanifold_edges();
    
  return EXIT_SUCCESS;
}


// --------- test the get_environment_vertices function ------- //
void test_environment_vertices(const pmp::SurfaceMesh& mesh)
{
  // compare get_environment_vertices with a direct identification
  auto v_container = mesh.vertices();
  for(auto v:v_container)
    if(!mesh.is_boundary(v))
      {
	auto v_ring = vm::get_environment_vertices(v, mesh);
	std::set<int> vec1{};
	for(auto it:v_ring)
	  {
	    auto result = vec1.insert(it.idx());
	    if(result.second==false)
	      {
		std::cerr << "\ntest_environment_vertices: duplication in vertices\n";
		std::exit(EXIT_FAILURE);
	      }
	  }
	
	auto f_circulator = mesh.faces(v);
	std::set<pmp::Edge> unique_edges{};
	for(auto f:f_circulator)
	  {
	    auto h_edges = mesh.halfedges(f);
	    for(auto h:h_edges)
	      {
		auto e = mesh.edge(h);
		auto [it, inserted] = unique_edges.insert(e);
		if(inserted==false)
		  unique_edges.erase(it);
	      }
	  }
	std::set<int> vec2{};
	for(auto e:unique_edges)
	  {
	    vec2.insert(mesh.vertex(e, 0).idx());
	    vec2.insert(mesh.vertex(e, 1).idx());
	  }

	if(vec1!=vec2)
	  {
	    std::cerr << "\ntest_environment_vertices failed\n" << std::flush;
	    std::exit(EXIT_FAILURE);
	  }
      }
}
  

// ---------  test the environment polygon -------- //
void test_environment_polygon(const pmp::SurfaceMesh& mesh)
{
  auto v_container = mesh.vertices();
  for(auto v:v_container)
    if(!mesh.is_boundary(v))
      {
	auto env_poly = vm::get_environment_polygon(v, mesh);

	// faces incident at v
	std::vector<vm::boost_polygon_t> face_polygons{};
	auto f_circulator = mesh.faces(v);
	for(auto f:f_circulator)
	  {
	    auto face_verts = mesh.vertices(f);
	    std::vector<pmp::Point> coords{};
	    for(auto w:face_verts)
	      coords.push_back(mesh.position(w));
	    
	    face_polygons.push_back(vm::make_polygon(coords));
	  }

	// polygonal union of faces
	vm::bgm::multi_polygon<vm::boost_polygon_t> faces_union;
	for (const auto& f:face_polygons)
	  {
	    vm::bgm::multi_polygon<vm::boost_polygon_t> tmp;
	    vm::bg::union_(faces_union, f, tmp);
	    faces_union = std::move(tmp);
	  }

	// env_poly = union of faces around v.
	vm::bgm::multi_polygon<vm::boost_polygon_t> symdiff;
	vm::bg::sym_difference(env_poly, faces_union, symdiff);
	double diff_area = 0.0;
	for (const auto& p : symdiff)
	  diff_area += std::abs(vm::bg::area(p));
	if(diff_area>vm::bg::area(env_poly)*1.e-4)
	  {
	    std::cerr << "\ntest_environment_polygon: union check failed\n" << std::flush;
	    std::exit(EXIT_FAILURE);
	  }
      }
}


// ---------  test erase non manifold edges -------- //
void test_erase_nonmanifold_edges()
{
  using pmp::Vertex;
  using pmp::Point;
  using vm::erase_nonmanifold_edges;

  // Five vertices with real 3D coordinates. actual geometry is unimportant
  pmp::SurfaceMesh m;
  const Vertex A = m.add_vertex(Point(0.0,  0.0, 0.0));
  const Vertex B = m.add_vertex(Point(1.0,  0.0, 0.0));
  const Vertex C = m.add_vertex(Point(1.0,  1.0, 0.0));
  const Vertex D = m.add_vertex(Point(0.0,  1.0, 0.0));
  const Vertex E = m.add_vertex(Point(-1.0, 0.5, 0.0));

  auto fail = [](const char* tag) {
    std::cerr << "\ntest_erase_nonmanifold_edges[" << tag << "] failed\n" << std::flush;
    std::exit(EXIT_FAILURE);
  };

  auto check_equal = [&](std::vector<Vertex> got,
                         const std::vector<Vertex>& expected,
                         const char* tag) {
    if (got.size() != expected.size()) fail(tag);
    for (std::size_t i = 0; i < got.size(); ++i)
      if (got[i].idx() != expected[i].idx()) fail(tag);
  };

  auto check_throws = [&](std::vector<Vertex> input, const char* tag) {
    bool threw = false;
    try { erase_nonmanifold_edges(input); }
    catch (const std::runtime_error&) { threw = true; }
    if (!threw) fail(tag);
  };

  // (1) Manifold ring — untouched
  {
    std::vector<Vertex> v{A, B, C, D};
    erase_nonmanifold_edges(v);
    check_equal(v, {A, B, C, D}, "manifold-4");
  }

  // (2) Interior non-manifold: [A,B,C,B,D,E] -> [A,B,D,E]
  //     v[1]==v[3] both B; match detected at it=C(2),
  //     erases C(2) and prev B(1), leaving the later B at original index 3.
  {
    std::vector<Vertex> v{A, B, C, B, D, E};
    erase_nonmanifold_edges(v);
    check_equal(v, {A, B, D, E}, "interior-pair");
  }
 
  // (3) Wrap-around detected at it=begin: [A,B,C,D,E,B] -> [B,C,D,E]
  //     v[0]==A, v[1]==v[5]==B; circular_previous(begin)=v[5], next=v[1].
  {
    std::vector<Vertex> v{A, B, C, D, E, B};
    erase_nonmanifold_edges(v);
    check_equal(v, {B, C, D, E}, "wrap-around");
  }

  // (4) Cascading non-manifolds: [A,B,C,B,A,D,E] -> [A,D,E]
  //     pass 1 erases C+B (it=C), leaving [A,B,A,D,E];
  //     pass 2 erases B+A,        leaving [A,D,E].
  {
    std::vector<Vertex> v{A, B, C, B, A, D, E};
    erase_nonmanifold_edges(v);
    check_equal(v, {A, D, E}, "cascade");
  }

  // (5) Degenerate ring throws: [A,B,A] reduces to size 1
  check_throws({A, B, A},    "degenerate-3");

  // (6) Degenerate ring throws: [A,B,A,B] reduces to size 2
  check_throws({A, B, A, B}, "degenerate-4");
}
