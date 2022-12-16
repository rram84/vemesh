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

  
  // longest half edge incident at a vertex
  double compute_longest_halfedge_length_at_vertex(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex)
  {
    double longest_h  = std::numeric_limits<double>::min();
    auto h_circulator = mesh.halfedges(vertex);
    const auto& X     = mesh.position(vertex);
    for(auto h:h_circulator)
      {
	const auto& Y = mesh.position(mesh.to_vertex(h));
	double len    = std::sqrt((X[0]-Y[0])*(X[0]-Y[0])+(X[1]-Y[1])*(X[1]-Y[1]));
	if(len>longest_h)
	  longest_h = len;
      }
    return longest_h;
  }

  
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

    assert(compute_flag==true);
    
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

  std::cout << "h1 = " << h1 << ", h2 = " << h2 << std::endl;
    if(h1<h2)
      return h1;
    else
      return h2;
  }
  
  
  // measure
  // (i)  minimum distance of the vertex to its enclosing linestring
  // (ii) minimum visible-orthogonal-distance of ring vertices to half-edges emanating from the vertex
  // defined only for non-boundary vertices, not connected to hanging nodes
  /*

  // identify the list of vertices in the ring that are not connected to the vertex
  std::vector<pmp::Vertex> unconnected_vertex_ring(const pmp::SurfaceMesh& mesh,
  const pmp::Vertex& vertex,
  const std::vector<pmp::Vertex>& vertex_ring)
  {
  // set of ring vertices
  auto cmp = [](const pmp::Vertex& A, const pmp::Vertex& B) { return A.idx()<B.idx(); };
  std::set<pmp::Vertex, decltype(cmp)> ring_set(cmp);
				  for(auto& v:vertex_ring)
				  ring_set.insert(v);
				  assert(ring_set.size()==vertex_ring.size());

				  // set of half-edge connected vertices
				  std::set<pmp::Vertex, decltype(cmp)> conn_set(cmp);
				  auto h_circulator = mesh.halfedges(vertex);
				  for(auto h:h_circulator)
				  conn_set.insert(mesh.to_vertex(h));

				  // set of unconnected ring vertices;
				  std::vector<pmp::Vertex> unconn{};
				  std::set_difference(ring_set.begin(), ring_set.end(),
				  conn_set.begin(), conn_set.end(),
				  std::back_inserter(unconn));

				  assert(vertex_ring.size()==unconn.size()+conn_set.size());
				  return std::move(unconn);
				  }



				  double compute_distance_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert)
				  {
				  // get the ring of vertices around this vertex
				  const std::vector<pmp::Vertex> vertex_ring = get_vertex_ring(mesh, vert);

    
    
				  // longest halfedge at this vertex
				  double max_h = min_dist;
    
				  // linestrings of halfedges
				  std::vector<boost_linestring_t> ls_halfedges{};

				  // list of connected vertices
				  std::vector<pmp::Vertex> connected_vertex_ring{};
    
				  auto h_circulator = mesh.halfedges(vert);
				  for(auto h:h_circulator)
				  {
				  // from = V, to = X
				  connected_vertex_ring.push_back(mesh.to_vertex(h));
				  const auto& X = mesh.position(mesh.to_vertex(h));
	
				  // this edge length
				  double len = std::sqrt((V[0]-X[0])*(V[0]-X[0]) + (V[1]-X[1])*(V[1]-X[1]));
				  if(len>max_h) max_h = len;

				  // append set of linestrings
				  ls_halfedges.push_back(boost_linestring_t{{V[0],V[1]},{X[0],X[1]}});
				  }

				  // ring vertices unconnected to "vertex"
				  std::vector<pmp::Vertex> unconnected_vertex_ring{};
				  for(auto& v:vertex_ring)
				  if(std::find(connected_vertex_ring.begin(), connected_vertex_ring.end(), v)==connected_vertex_ring.end())
				  unconnected_vertex_ring.push_back(v);

				  // measure the min distance of unconnected vertices to the halfedges
				  double h_dist = max_h;
				  for(auto& v:unconnected_vertex_ring)
				  {
				  const auto& X = mesh.position(v);
				  const boost_point_t  pt(X[0],X[1]);
				  for(auto& h:ls_halfedges)
				  {
				  const double mydist = bg::distance(pt, h);
				  if(mydist<h_dist)
				  h_dist = mydist;
				  }
				  }	
    
				  // return the ratio min(h_dist, min_dist)/max_h
				  if(h_dist<min_dist)
				  return h_dist/max_h;
				  else
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
				  */  
				  }
