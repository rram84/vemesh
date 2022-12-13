// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <utility>

namespace vm
{
  // compute the angle included by the pair of segments joining three points
  /*
   * u    w
   * \  /
   *  v
   */
  double compute_included_angle_in_degrees(const pmp::Point& U, const pmp::Point& V, const pmp::Point& W);

  
  // measure quality of a face as the smallest included angle
  double compute_angle_based_face_quality(const pmp::SurfaceMesh& mesh, const pmp::Face& face);
  double compute_angle_based_face_quality(const std::vector<pmp::Point>& coords);
  
  // measure quality as the ratio of the distance of a vertex to its enclosing linestring to the longest halfedge
  // defined only for non-boundary vertices, not connected to hanging nodes
  double compute_distance_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);

  // measure quality as the smallest included face angle among faces incident at a vertex
  // define only for non-boundary vertices, not connected to hanging nodes
  double compute_angle_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);
}
