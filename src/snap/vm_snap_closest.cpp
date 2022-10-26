// Sriramajayam

#include <vm_snap.h>

// boost utilities
#include <boost/geometry/geometry.hpp>

namespace vm
{
  // boost aliases
  namespace bg  = boost::geometry;
  namespace bgm = bg::model;
  using boost_point_t         = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_segment_t       = bgm::segment<boost_point_t>;
  
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

}
