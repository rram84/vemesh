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
  using boost_linestring_t    = bgm::linestring<boost_point_t>;

  // determine whether the vertex of a face needs to be snapped
  std::vector<pmp::Vertex> needs_snap(const pmp::SurfaceMesh& mesh, const pmp::Face& face, const double eps_dist_ratio)
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

}
