// Sriramajayam

#include <vm_utils.h>
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
  using boost_multi_polygon_t  = bgm::multi_polygon<boost_polygon_t>;

  bool inspect_mesh(const pmp::SurfaceMesh &mesh)
  {
    assert(mesh.n_vertices()>0);
    assert(mesh.n_faces()>0);
    assert(mesh.n_edges()>0);
    
    // translate to boost polygons
    std::map<int, boost_polygon_t> polygons{};
    auto face_circulator = mesh.faces();
    for(auto face:face_circulator)
      {
	// vertices of this face
	auto vertex_circulator = mesh.vertices(face);
	
	// boost polygon representation
	boost_polygon_t poly;
	for(auto v:vertex_circulator)
	  {
	    const auto& pt = mesh.position(v);
	    bg::append(poly.outer(), boost_point_t(pt[0],pt[1]));
	  }
	auto first_vertex = *poly.outer().begin();
	bg::append(poly.outer(), first_vertex);
	polygons.insert({face.idx(), poly});
      }

    bool flag = true;
    
    // each face should be valid, simple and have a positive area
    for(auto& it:polygons) {

      const auto& findx = it.first;
      const auto& poly = it.second;
      
      std::string message;
      if(!bg::is_valid(poly, message))
	{
	  std::cout << "Mesh inspection failed for face " << findx << std::endl
		    << "face is not valid, with message: " << message << std::endl
		    << "Polygon: " << boost::geometry::wkt(poly) << std::endl;
	  flag = false;
	}
      if(!bg::is_simple(poly))
	{
	  std::cout << "Mesh inspection failed for face " << findx << std::endl
		    << "face is not simple " << std::endl
		    << "Polygon: " << boost::geometry::wkt(poly) << std::endl;
	  flag = false;
	}
      if(bg::area(poly)<=0.)
	{
	  std::cout << "Mesh inspection failed for face " << findx << std::endl
		    << "face area is negative " << std::endl
		    << "Polygon: " << boost::geometry::wkt(poly) << std::endl;
	  std::cout << "Not positive " << std::endl;
	  flag = false;
	}
    }
    
    // faces should not overlap with neighbors
    // check intersection with neighbors
    for(auto face:face_circulator) {

      // this face
      const auto& findx = face.idx();
      const auto& poly = polygons.at(findx);
      
      // area
      double area = bg::area(poly);
      
      // its neighbors
      auto halfedge_circulator = mesh.halfedges(face);
      for(auto h:halfedge_circulator)
	{
	  assert(mesh.is_valid(h));
	  assert(mesh.is_boundary(h)==false);  	// cannot be a boundary halfedge

	  // // nothing to do in case of no neighbor
	  auto h_opp  = mesh.opposite_halfedge(h);
	  if(mesh.is_boundary(h_opp)==true)
	    continue;

	  // neighboring face
	  auto nb_face = mesh.face(h_opp);
	  assert(mesh.is_valid(nb_face)==true);

	  // avoid double checking pairwise insersections
	  const int nb_findx = nb_face.idx();
	  if(findx>nb_findx)
	    continue;

	  // check pairwise intersection
	  const auto& nb_poly = polygons.at(nb_findx);
	  boost_multi_polygon_t intersection;
	  bool does_intersect = bg::intersection(poly, nb_poly, intersection);
	  assert(does_intersect==true); // at vertices and edges
	  double intersection_area = bg::area(intersection);

	  if(std::abs(intersection_area/area)>1.e-3) {
	    std::cout << "Invalid intersection of neighboring faces "
		      << findx << " and " << nb_findx << " overlap. " << std::endl
		      << "Face " << findx << ": " << boost::geometry::wkt(poly) << std::endl
		      << "Face " << nb_findx << ": " << boost::geometry::wkt(nb_poly) << std::endl;
	    flag = false;
	  }
	}
    }

    // done
    return flag;
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
