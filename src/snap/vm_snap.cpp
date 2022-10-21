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

} // vm::
