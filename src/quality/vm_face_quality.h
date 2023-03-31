// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <utility>
#include <list>

namespace vm
{
  // measure quality of a face as the smallest included angle
  double compute_angle_based_face_quality(const pmp::SurfaceMesh& mesh, const pmp::Face& face);
  double compute_angle_based_face_quality(const std::vector<pmp::Point>& coords);
  
  // compute the angle included by the pair of segments joining three points
  /*
   * u    w
   * \  /
   *  v
   */
  double compute_included_angle_in_degrees(const pmp::Point& U, const pmp::Point& V, const pmp::Point& W);


  // measure quality of a face as the smallest nonzero eigenvalue of the vem stiffness matrix
  double compute_stiffness_based_face_quality(const pmp::SurfaceMesh& mesh, const pmp::Face& face);
}
