// Sriramajayam

#include <vm_inspect.h>

// boost polygon utilities
#include <boost/geometry/geometry.hpp>

namespace vm
{
  // boost aliases
  namespace bg  = boost::geometry;
  namespace bgm = bg::model;
  using boost_point_t         = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_linestring_t    = bgm::linestring<boost_point_t>;
  using boost_polygon_t       = bgm::polygon<boost_point_t, false>;
  using boost_multi_polygon_t = bgm::multi_polygon<boost_polygon_t>;

  // List of checks:
  // (i)   all faces around the vertex should be simple
  // (ii)  all faces around the vertex should have positive area
  // (iii) pairwise intersections should be empty

  // inspect correctness of a vertex ring
  bool inspect_vertex_ring(const pmp::SurfaceMesh& mesh,
			   const pmp::Vertex& vertex)
  {
    // boost polygons for all faces
    std::vector<boost_polygon_t> face_poly{};
    double total_area = 0.;
    
    auto f_circulator = mesh.faces(vertex);
    for(auto f:f_circulator)
      {
	boost_polygon_t    poly;
	boost_linestring_t ls;
	auto v_circulator = mesh.vertices(f);

	for(auto v:v_circulator)
	  {
	    const auto& X = mesh.position(v);
	    bg::append(poly.outer(), boost_point_t(X[0],X[1]));
	    bg::append(ls, boost_point_t(X[0],X[1]));
	  }

	// repeat the first point
	for(auto v:v_circulator)
	  {
	    const auto& X = mesh.position(v);
	    bg::append(poly.outer(), boost_point_t(X[0],X[1]));
	    bg::append(ls, boost_point_t(X[0],X[1]));
	    break;
	  }

	// does this face have positive area
	double poly_area = bg::area(poly);
	assert(poly_area>0.);
	total_area += poly_area;
	
	// is this face simple
	assert(bg::is_simple(poly)==true);
	assert(bg::is_simple(ls)==true);

	// append the polygon representation of this face
	face_poly.push_back(poly);
      }  

    // pairwise intersections of faces should be empty
    const int nfaces = static_cast<int>(face_poly.size());
    for(int i=0; i<nfaces; ++i)
      for(int j=i+1; j<nfaces; ++j)
	{
	  boost_multi_polygon_t diff;
	  bg::sym_difference(face_poly[i], face_poly[j], diff);
	  assert(std::abs(bg::area(diff))/total_area<1.e-3);
	}
      
    // done
    return true;
  }
}
  
