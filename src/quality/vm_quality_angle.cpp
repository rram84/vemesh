// Sriramajayam

#include <vm_quality.h>
#include <cmath>

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
}
