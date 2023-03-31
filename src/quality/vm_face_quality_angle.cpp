// Sriramajayam

#include <vm_face_quality.h>
#include <cmath>
#include <cassert>

namespace vm
{
  // compute the angle included by the pair of segments joining three points
  /*
   * u    w
   * \  /
   *  v
   */
  double compute_included_angle_in_degrees(const pmp::Point& U, const pmp::Point& V, const pmp::Point& W)
  {
    // edges
    const double VU[] = {U[0]-V[0], U[1]-V[1]};
    const double VW[] = {W[0]-V[0], W[1]-V[1]};

    // measure the angle at vertex V
    const double dot = VU[0]*VW[0] + VU[1]*VW[1];
    const double det = VU[0]*VW[1] - VU[1]*VW[0];
    double angle     = std::atan2(-det, dot);
    if(angle<0.)
      angle += 2.*M_PI;

    return (180./M_PI)*angle;
  }
  
  // measure quality of a face as the smallest included angle
  double compute_angle_based_face_quality(const pmp::SurfaceMesh& mesh, const pmp::Face& face)
  {
    // make a list of vertex coordinates
    std::vector<pmp::Point> vertices{};
    auto v_circulator = mesh.vertices(face);
    for(auto v:v_circulator)
      vertices.push_back( mesh.position(v) );

    return compute_angle_based_face_quality(vertices);
  }
  

  double compute_angle_based_face_quality(const std::vector<pmp::Point>& coords)
  {
    const int nverts = static_cast<int>(coords.size());
    double min_angle = 360.;
    for(int a=0; a<nverts; ++a)
      {
	const auto& Xa = coords[a];
	const auto& Xb = coords[(a+1)%nverts];
	const auto& Xc = coords[(a+2)%nverts];

	// angle between edges ab and bc
	const double angle = compute_included_angle_in_degrees(Xa, Xb, Xc);

	// track the minimum
	if(angle<min_angle)
	  min_angle = angle;
      }
    
    return min_angle;
  }

}
