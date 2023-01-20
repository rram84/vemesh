// Sriramajayam

#include <vm_inspect.h>
#include <vm_vertex_ring.h>

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
  // (i)   given environment should be simple
  // (ii)  visibility polygon should be simple
  // (iii) visibility polygon should have finite area
  // (iv)  visibility polygon should be contained within the environment provided
  // (iv)  visibility polygon should contain the vertex provided
  
  // inspect the correctness of a computed visibility polygon
  bool inspect_visibility_polygon(const pmp::SurfaceMesh& mesh,
				  const pmp::Vertex& vertex,
				  const std::vector<std::pair<double,double>>& vis_poly_verts)
  {
    // environment
    const auto vertex_ring = get_vertex_ring(mesh, vertex);

    // boost polygon and linstring representation for the environment
    boost_polygon_t    env_poly;
    boost_linestring_t env_ls;
    const int num_env_verts = static_cast<int>(vertex_ring.size());
    for(int i=0; i<num_env_verts+1; ++i)
      {
	const auto& v = vertex_ring[(i+1)%num_env_verts];
	const auto& X = mesh.position(v);
	bg::append(env_poly.outer(), boost_point_t(X[0],X[1]));
	bg::append(env_ls, boost_point_t(X[0],X[1]));
      }

    // environment should have positive area
    assert(bg::area(env_poly)>0.);

    // environment should be simple
    assert(bg::is_simple(env_poly)==true);
    assert(bg::is_simple(env_ls)==true);

    // boost polygon and linestring representation for the visibility polygon
    boost_polygon_t    vis_poly;
    boost_linestring_t vis_ls;
    const int num_vis_verts = static_cast<int>(vis_poly_verts.size());
    for(int i=0; i<num_vis_verts+1; ++i)
      {
	const auto& X = vis_poly_verts[(i+1)%num_vis_verts];
	bg::append(vis_poly.outer(), boost_point_t(X.first, X.second));
	bg::append(vis_ls, boost_point_t(X.first, X.second));
      }

    // polygon should have positive area
    double vis_poly_area = bg::area(vis_poly);
    assert(vis_poly_area>0.);

    // polygon should be simple
    assert(bg::is_simple(vis_poly)==true);
    assert(bg::is_simple(vis_ls)==true);

    // visibility polygon should lie inside the environment
    // equivalently, vis_poly-env_poly should have zero area
    boost_multi_polygon_t vis_minus_env;
    bg::difference(vis_poly, env_poly, vis_minus_env);
    assert(std::abs(bg::area(vis_minus_env))/vis_poly_area<1.e-3);

    // the given vertex should lie inside the visibility polygon
    const auto& Xv = mesh.position(vertex);
    assert(bg::within(boost_point_t(Xv[0], Xv[1]), vis_poly)==true);
    
    return true;
  }
  
}
