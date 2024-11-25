// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <utility>
#include <list>
#include <Eigen/Dense>

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
  //double compute_included_angle_in_degrees(const pmp::Point& U, const pmp::Point& V, const pmp::Point& W);


  Eigen::MatrixXd compute_polygon_stiffness_matrix(const std::vector<pmp::Point>& coords,
						   const double stabilization = 1.0);
  
  // measure quality of a face as the smallest nonzero eigenvalue of the vem stiffness matrix
  double compute_stiffness_based_mesh_face_quality(const pmp::SurfaceMesh& mesh, const pmp::Face& face);
  double compute_stiffness_based_face_quality(const std::vector<pmp::Point>& coords);
  
  // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
  // smallest nonzero eigenvalue
  double compute_stiffness_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);

  // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
  // ratio of the area to the perimeter^2
  double compute_shape_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);
}
