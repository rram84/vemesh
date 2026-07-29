// Sriramajayam

#include <vm_quality.h>
#include <cmath>
#include <cassert>
#include <limits>

namespace vm
{
  // measure the ratio of smallest and largest edge lengths at a vertex
  double min_max_edge_length_ratio_at_vertex(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert)
  {
    assert(mesh.is_valid(vert)==true);
    
    // minimum and maximum edge lengths
    double min_edge_len = std::numeric_limits<double>::max();
    double max_edge_len = std::numeric_limits<double>::min();

    // coordinates of vertex
    const auto& X = mesh.position(vert);

    // loop over half-edges at vert
    auto halfedge_circulator = mesh.halfedges(vert);
    for(auto h:halfedge_circulator)
      {
	const auto& Y = mesh.position(mesh.to_vertex(h));

	// compare edge lengths
	const double edge_len = std::sqrt((X[0]-Y[0])*(X[0]-Y[0])+(X[1]-Y[1])*(X[1]-Y[1]));
	if(edge_len<min_edge_len)
	  min_edge_len = edge_len;
	if(edge_len>max_edge_len)
	  max_edge_len = edge_len;
      }

    return min_edge_len/max_edge_len;
  }

  // measure the smallest face angle at a vertex
  double min_face_angle_at_vertex(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert)
  {
    assert(mesh.is_valid(vert)==true);

    // this vertex
    const auto& V = mesh.position(vert);

    // min face angle incident at the vertex
    double min_angle = std::numeric_limits<double>::max();

    // iterate over faces
    auto face_circulator = mesh.faces(vert);
    for(auto face:face_circulator)
      {
	// identify the half-edges meeting at "vert"
	auto h_circulator = mesh.halfedges(face);
	pmp::Halfedge h_from;
	for(auto h:h_circulator)
	  if(mesh.from_vertex(h)==vert)
	    {
	      h_from = h;
	      break;
	    }
	auto h_to = mesh.prev_halfedge(h_from);

	// compute the included angle between h_from (vw) and h_to (uv)
	/*
	 * u    w
	 * \  /
	 *  v
	 */
	const auto& U = mesh.position(mesh.from_vertex(h_to));
	const auto& W = mesh.position(mesh.to_vertex(h_from));

	// edges
	const double VU[] = {U[0]-V[0], U[1]-V[1]};
	const double VW[] = {W[0]-V[0], W[1]-V[1]};

	// measure the angle at vertex V
	const double dot = VU[0]*VW[0] + VU[1]*VW[1];
	const double det = VU[0]*VW[1] - VU[1]*VW[0];
	double angle     = std::atan2(-det, dot);
	if(angle<0.)
	  angle += 2.*M_PI;

	if(angle<min_angle)
	  min_angle = angle;
      }

    // done
    return (180./M_PI)*min_angle;
  }


  // measure quality of a vertex as
  // (i) the smallest included angle
  // (i) the ratio of smallest and largest edge lengths
  std::pair<double,double> vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert)
  {
    const double edge_ratio = min_max_edge_length_ratio_at_vertex(mesh, vert);
    const double theta      = min_face_angle_at_vertex(mesh, vert);
    assert(edge_ratio>0. && edge_ratio<1.);
    assert(theta>0. && theta<360.);
    
    return {edge_ratio, theta};
  }
  
}
