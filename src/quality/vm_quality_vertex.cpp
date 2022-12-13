// Sriramajayam

#include <vm_vertex_ring.h>
#include <vm_quality.h>
#include <cmath>
#include <cassert>
#include <limits>

// boost polygon utilities
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace vm
{
  // boost aliases
  namespace bg             = boost::geometry;
  namespace bgm            = bg::model;
  using boost_point_t      = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_linestring_t = bgm::linestring<boost_point_t>;
  
  // measure quality as the ratio of the distance of a vertex to its enclosing linestring to the longest halfedge
  // defined only for non-boundary vertices, not connected to hanging nodes
  double compute_distance_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert)
  {
    // get the ring of vertices around this vertex
    const std::vector<pmp::Vertex> vertex_ring = get_vertex_ring(mesh, vert);

    // linestring representing the polygon ring around vert
    boost_linestring_t ls;
    for(auto& v:vertex_ring)
      {
	const auto& X = mesh.position(v);
	bg::append(ls, boost_point_t(X[0],X[1]));
      }
    // repeat the first vertex
    {
      const auto& X = mesh.position(vertex_ring.front());
      bg::append(ls, boost_point_t(X[0],X[1]));
    }
    bg::correct(ls);
    
    // distance of vertex from the linestring
    const auto& V = mesh.position(vert);
    const double min_dist  = bg::distance(boost_point_t(V[0], V[1]), ls);

    // length of the longest halfedge at this vertex
    auto h_circulator = mesh.halfedges(vert);
    double max_h = min_dist;
    for(auto h:h_circulator)
      {
	const auto& X = mesh.position(mesh.to_vertex(h));
	double len = std::sqrt((V[0]-X[0])*(V[0]-X[0]) + (V[1]-X[1])*(V[1]-X[1]));
	if(len>max_h)
	  max_h = len;
      }

    // return the ratio
    return min_dist/max_h;
  }


  
    
  
  // measure quality as the smallest included face angle among faces incident at a vertex
  // defined only for non-boundary vertices, not connected to hanging nodes
  double compute_angle_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex)
  {
    // loop over all faces
    // examine included angles at three vertices- v, and its connected vertices
    double min_angle = 360.;
    auto f_circulator = mesh.faces(vertex);
    for(auto face:f_circulator)
      {
	// halfedges of this face
	auto h_circulator = mesh.halfedges(face);

	// loop around to find the "from" and "to" halfedges
	pmp::Halfedge h_from, h_to;
	for(auto h:h_circulator)
	  {
	    if(mesh.to_vertex(h)==vertex)
	      {
		h_to   = h;
		h_from = mesh.next_halfedge(h);
		break;
	      }
	    else if(mesh.from_vertex(h)==vertex)
	      {
		h_from = h;
		h_to   = mesh.prev_halfedge(h);
		break;
	      }
	  }

	// find the included angle at vertices
	const auto& T = mesh.position(mesh.from_vertex(mesh.prev_halfedge(h_to)));
	const auto& U = mesh.position(mesh.from_vertex(h_to));
	const auto& V = mesh.position(vertex);
	const auto& W = mesh.position(mesh.to_vertex(h_from));
	const auto& X = mesh.position(mesh.to_vertex(mesh.next_halfedge(h_from)));

	const double angle_U = compute_included_angle_in_degrees(T, U, V);
	const double angle_V = compute_included_angle_in_degrees(U, V, W);
	const double angle_W = compute_included_angle_in_degrees(V, W, X);

	// track the minimum
	if(angle_U < min_angle) min_angle = angle_U;
	if(angle_V < min_angle) min_angle = angle_V;
	if(angle_W < min_angle) min_angle = angle_W;
      }

    // done
    return min_angle;
  }
  
}
