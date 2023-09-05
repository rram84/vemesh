// Sriramajayam

#include <vm_inspect.h>
#include <cassert>

// boost polygon utilities
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace vm
{
  // boost aliases
  namespace bg  = boost::geometry;
  namespace bgm = bg::model;
  using boost_point_t    = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_polygon_t  = bgm::polygon<boost_point_t, false>;

  bool inspect_face(const pmp::SurfaceMesh& mesh, pmp::Face& face)
  {
    assert(mesh.is_valid(face)==true && mesh.is_deleted(face)==false);
    
    // vertices of this face
    auto vertex_circulator = mesh.vertices(face);

    // boost polygon representation
    boost_polygon_t poly;
    for(auto v:vertex_circulator)
      {
	const auto& pt = mesh.position(v);
	bg::append(poly.outer(), boost_point_t(pt[0],pt[1]));
      }
    auto first_vert = poly.outer().begin();
    bg::append(poly.outer(), boost_point_t(bg::get<0>(*first_vert), bg::get<1>(*first_vert)));

    // the face should be valid, simple and have a positive area
    if(!bg::is_valid(poly))
      return false;
    if(!bg::is_simple(poly))
      return false;
    if(bg::area(poly)<=0.)
      return false;
    
    // check intersection with neighbors
    auto halfedge_circulator = mesh.halfedges(face);
    for(auto h:halfedge_circulator)
      {
	assert(mesh.is_valid(h));

	// cannot be a boundary halfedge
	assert(mesh.is_boundary(h)==false);

	// opposite half-edge
	auto h_opp  = mesh.opposite_halfedge(h);

	// nothing to do in case of no neighbor
	if(mesh.is_boundary(h_opp)==true)
	  continue;

	// neighboring face
	auto nb_face = mesh.face(h_opp);
	assert(mesh.is_valid(nb_face)==true);
      
	// vertices of neighboring face
	auto nb_vertex_circulator = mesh.vertices(nb_face);
      
	// polygon representation of neighboring face
	boost_polygon_t nb_poly;
	for(auto v:nb_vertex_circulator)
	  {
	    const auto& pt = mesh.position(v);
	    bg::append(nb_poly.outer(), boost_point_t(pt[0],pt[1]));
	  }
	auto nb_first_vert = nb_poly.outer().begin();
	bg::append(nb_poly.outer(), boost_point_t(bg::get<0>(*nb_first_vert), bg::get<1>(*nb_first_vert)));

	// neighbor should be valid & simple
	if(!bg::is_valid(nb_poly))
	  return false;
	if(!bg::is_simple(nb_poly))
	  return false;

	// intersection of neighboring faces should be an edge
	std::vector<boost_polygon_t> intersection{};
	bg::intersection(poly, nb_poly, intersection);
	if(intersection.empty()==false)
	  return false;
      }

    // done
    return true;
  }


  bool inspect_face(const std::vector<pmp::Point>& coords)
  {
    // boost polygon representation
    boost_polygon_t poly;
    for(const auto& pt:coords)
      bg::append(poly.outer(), boost_point_t(pt[0],pt[1]));

    // close the loop
    bg::append(poly.outer(), boost_point_t(coords[0][0], coords[0][1]));
    
    // the face should be valid & simple
    return (bg::is_valid(poly) && bg::is_simple(poly));
  }

}
