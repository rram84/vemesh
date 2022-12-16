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
  using boost_polygon_t    = bgm::polygon<boost_point_t>;

  
  // minimum distance of a vertex from its connected ring
  double compute_minimum_vertex_to_ring_distance(const pmp::SurfaceMesh& mesh,
						 const pmp::Vertex& vertex)
  {
    // get the ring of vertices around this vertex
    const std::vector<pmp::Vertex> vertex_ring = get_vertex_ring(mesh, vertex);

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

    // return distance of the vertex from the linestring
    const auto& V = mesh.position(vertex);
    return bg::distance(boost_point_t(V[0], V[1]), ls);
  }
  


  // measure the minimum visible altitude of face vertices to a given halfedge
  std::pair<bool, double> compute_minimum_visible_altitude_to_halfedge(const pmp::SurfaceMesh& mesh,
								       const pmp::Halfedge& h)
  {
    // this face
    auto face = mesh.face(h);

    // vertices
    auto v_circulator = mesh.vertices(mesh.face(h));

    // boost polygon
    boost_polygon_t poly;
    for(auto v:v_circulator)
      {
	const auto& X = mesh.position(v);
	bg::append(poly.outer(), boost_point_t(X[0],X[1]));
      }
    // repeat the first vertex
    for(auto v:v_circulator)
      {
	const auto& X = mesh.position(v);
	bg::append(poly.outer(), boost_point_t(X[0],X[1]));
	break;
      }
    bg::correct(poly);

    // vertices to exclude when computing altitude
    double min_altitude  = std::numeric_limits<double>::max();
    bool compute_flag    = false;
    const pmp::Vertex vA = mesh.from_vertex(h);
    const pmp::Vertex vB = mesh.to_vertex(h);
    const auto& xA       = mesh.position(vA);
    const auto& xB       = mesh.position(vB);
    const double tvec[]  = {xB[0]-xA[0], xB[1]-xA[1]};
    const double len2    = tvec[0]*tvec[0]+tvec[1]*tvec[1];
    double lambda, xD[2];
    for(auto v:v_circulator)
      if(v!=vA && v!=vB)
	{
	  const auto& xC = mesh.position(v);

	  // orthogonal projection of xC on xA-xB
	  lambda = ((xC[0]-xA[0])*tvec[0] + (xC[1]-xA[1])*tvec[1])/len2;
	  
	  // does D lie on the segment
	  if(lambda<=0 || lambda>=1)
	    continue;
	  

	  // does the segment CD lie within this polygon
	  boost_linestring_t segCD{ {xC[0],xC[1]}, {xA[0]+lambda*tvec[0],xA[1]+lambda*tvec[1]} };
	  if(bg::within(segCD,poly))
	    {
	      // check the minimum altitude
	      double alt = bg::length(segCD);
	      if(alt<min_altitude)
		{
		  min_altitude = alt;
		  compute_flag = true; 
		}
	    }
	}

    // done
    return {compute_flag, min_altitude};
  }


  // compute the minimum distance of ring vertices to the edges incident at a vertex
  double compute_minimum_ring_vertices_to_inner_halfedges_distance(const pmp::SurfaceMesh& mesh,
								   const pmp::Vertex& vertex)
								   
  {
    // half-edges incident at the vertex
    auto h_circulator = mesh.halfedges(vertex);

    // for each halfedge, examine the two faces on either side
    // determine the smallest visible altitude
    double min_altitude = std::numeric_limits<double>::max();
    bool compute_flag   = false;
    std::pair<bool, double> trial_altitude;
    for(auto h:h_circulator)
      {
	// this half edge
	trial_altitude = compute_minimum_visible_altitude_to_halfedge(mesh, h);
	if(trial_altitude.first==true && trial_altitude.second<min_altitude)
	  {
	    min_altitude = trial_altitude.second;
	    compute_flag = true;
	  }

	// its opposite halfedge
	auto h_opp = mesh.opposite_halfedge(h);
	trial_altitude = compute_minimum_visible_altitude_to_halfedge(mesh, h_opp);
	if(trial_altitude.first==true && trial_altitude.second<min_altitude)
	  {
	    min_altitude = trial_altitude.second;
	    compute_flag = true;
	  }
      }

    // HACK HACK HACK: HOW CAN THIS HAPPEN?
    //assert(compute_flag==true);
    
    // done
    return min_altitude;
  }


  // compute the minimum of vertex-to-ring distance, and ring-to-inner-edge distance
  double compute_distance_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex)
  {
    // minimum distance of a vertex from its connected ring
    const double h1 = compute_minimum_vertex_to_ring_distance(mesh, vertex);

    // minimum distance of ring vertex to inner halfedges
    const double h2 = compute_minimum_ring_vertices_to_inner_halfedges_distance(mesh, vertex);

    if(h1<h2)
      return h1;
    else
      return h2;
  }
  
}
