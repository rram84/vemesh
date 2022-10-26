// Sriramajayam

#include <vm_snap.h>

// boost utilities
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

namespace vm
{
  // boost aliases
  namespace bg  = boost::geometry;
  namespace bgm = bg::model;
  using boost_point_t         = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_segment_t       = bgm::segment<boost_point_t>;
  using boost_linestring_t    = bgm::linestring<boost_point_t>;
  
    
  // determine whether the vertex of a face needs to be snapped
  std::vector<pmp::Vertex> needs_snap(pmp::SurfaceMesh& mesh, const pmp::Face& face, const double eps_dist_ratio)
  {
    std::vector<pmp::Vertex> result{};

    // loop over vertices of the face
    // examine if the vertex can be snapped onto an edge it does not belong to
    auto halfedge_circulator = mesh.halfedges(face);
    const int valence        = mesh.valence(face);
    for(auto h:halfedge_circulator)
      {
	// vertex to examine
	auto vert = mesh.from_vertex(h);
	auto X    = mesh.position(vert);

	// min edge length of face at this vertex
	auto Y = mesh.position(mesh.to_vertex(h));
	auto Z = mesh.position(mesh.from_vertex(mesh.prev_halfedge(h)));
	const double L1 = std::sqrt((X[0]-Y[0])*(X[0]-Y[0]) + (X[1]-Y[1])*(X[1]-Y[1]));
	const double L2 = std::sqrt((X[0]-Z[0])*(X[0]-Z[0]) + (X[1]-Z[1])*(X[1]-Z[1]));
	const double min_edge_length = (L1<L2) ? L1 : L2;

	// linestring edges in the face complementary to X
	boost_linestring_t linestring;
	auto hedge = h;
	for(int count=0; count<valence-1; ++count)
	  {
	    const auto& pt = mesh.position(mesh.to_vertex(hedge));
	    linestring.push_back( boost_point_t(pt[0],pt[1]) );
	    hedge = mesh.next_halfedge(hedge);
	  }
	assert(linestring.size()==valence-1);

	// distance of X to the linestring
	const double dist  = bg::distance(boost_point_t(X[0],X[1]), linestring);
	
	// is the ratio small?
	if(dist/min_edge_length < eps_dist_ratio)
	  result.push_back(vert);
      }

    return result;
  }


  // identify the halfedge of a face closest to the given vertex
  pmp::Halfedge closest_halfedge(pmp::SurfaceMesh& mesh, const pmp::Face& face, const pmp::Vertex& vertex)
  {
    // location of the vertex
    const auto& Xv = mesh.position(vertex);
    const boost_point_t pt(Xv[0],Xv[1]);

    // track the closest distance
    double min_distance = std::numeric_limits<double>::max();
    
    // identify the halfedge of face closest to the vertex
    pmp::Halfedge closest_halfedge;
    auto halfedge_circulator = mesh.halfedges(face);

    for(auto h:halfedge_circulator)
      {
	auto a = mesh.from_vertex(h);
	auto b = mesh.to_vertex(h);
	if(a==vertex || b==vertex)    // vertex lies on this halfedge
	  continue;
	else                          // find the closest distance of 
	  {
	    const auto& Xa = mesh.position(a);
	    const auto& Xb = mesh.position(b);
	    boost_segment_t seg(boost_point_t(Xa[0],Xa[1]), boost_point_t(Xb[0],Xb[1]));
	    double dist = bg::distance(pt, seg);
	    if(dist<min_distance)
	      {
		min_distance = dist;
		closest_halfedge = h;
	      }
	  }
      }

    // done
    return closest_halfedge;
  }


  // determine the orthogonal projection of a vertex on a halfedge
  std::pair<bool, pmp::Point> projection_on_halfedge(pmp::SurfaceMesh& mesh,
						     const pmp::Vertex& vertex,
						     const pmp::Halfedge& halfedge)
  {
    std::pair<bool, pmp::Point> result;
    result.first = false;
    
    // location of the vertex
    const auto& Xv = mesh.position(vertex);

    // coordinates of the halfedge vertices
    const auto& Xa = mesh.position(mesh.from_vertex(halfedge));
    const auto& Xb = mesh.position(mesh.to_vertex(halfedge));

    // length of this edge
    const double len = (Xa[0]-Xb[0])*(Xa[0]-Xb[0]) + (Xa[1]-Xb[1])*(Xa[1]-Xb[1]);

    // orthogonal projection
    const double lambda = ((Xv[0]-Xa[0])*(Xb[0]-Xa[0]) + (Xv[1]-Xa[1])*(Xb[1]-Xa[1]))/len;

    // is the closest point an orthogonal projection?
    if(lambda>0. && lambda<1.)
      {
	result.first = true;
	result.second[0] = Xa[0] + lambda*(Xb[0]-Xa[0]);
	result.second[1] = Xa[1] + lambda*(Xb[1]-Xa[1]);
      }

    // done
    return result;
  }


  
  // examine whether snapping a vertex to its closest point on a half-edge is legal
  bool is_snap_ok(pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex, const pmp::Halfedge& halfedge)
  {
    // (i) orthogonal projection should lie on the half edge
    // (ii) it should be legal to delete all edges around vertex

    // examine projection
    auto proj_result = projection_on_halfedge(mesh, vertex, halfedge);
    if(proj_result.first==false)
      return false;

    // edge removal
    auto halfedge_circulator = mesh.halfedges(vertex);
    for(auto h:halfedge_circulator)
      {
	auto edge = mesh.edge(h);
	assert(mesh.is_valid(edge));
	if(mesh.is_removal_ok(edge)==false)
	  return false;
      }

    // all ok
    return true;
  }


  // snap a vertex to its closest point on a halfedge
  void snap(pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex, const pmp::Halfedge& halfedge)
  {
    // vertices of the halfedge
    const auto& vA = mesh.from_vertex(halfedge);
    const auto& vB = mesh.to_vertex(halfedge);
    
    // get the closest point projection of vertex on the halfedge
    auto proj_result = projection_on_halfedge(mesh, vertex, halfedge);
    assert(proj_result.first==true);
    
    // split the halfedge by inserting a new vertex at the projection location
    auto new_halfedge = mesh.insert_vertex(mesh.edge(halfedge), proj_result.second);
    assert(mesh.from_vertex(new_halfedge)==mesh.from_vertex(halfedge));
    
    // list of outgoing halfedges around 'vertex'
    std::vector<pmp::Halfedge> old_halfedges{};
    auto halfedge_circulator = mesh.halfedges(vertex);
    for(auto h:halfedge_circulator)
      old_halfedges.push_back(h);

    // modify old_halfedges by replacing connections from 'vertex' to connections from 'new_vertex'
    // halfedges with tip equal to 'vA' and 'vB' are omitted
    for(auto& h:old_halfedges)
      {
	assert(mesh.from_vertex(h)==vertex);

	// bypass collapsed edges
	if(mesh.to_vertex(h)==vA || mesh.to_vertex(h)==vB)
	  continue;

	// non-collapsed edges
	
	// remove the old edge of 'h'
	auto edge = mesh.edge(h);
	assert(mesh.is_removal_ok(edge)==true);
	mesh.remove_edge(edge);

	// add the new edge in its place: to_vertex(h) --> to_vertex(new_halfedge)
	mesh.insert_edge(h, new_halfedge);
      }
    
    // done
    return;
  }
    
} // vm::
