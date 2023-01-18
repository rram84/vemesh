// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <utility>
#include <set>
#include <map>

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

  struct LimitCircle_t
  {
    double center[2];
    double radius;
  };
  
  // measure quality as the ratio of the distance of a vertex to its enclosing linestring to the longest halfedge
  // defined only for non-boundary vertices, not connected to hanging nodes
  LimitCircle_t compute_distance_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);

  // compute the minimum distance of ring vertices to the edges incident at a vertex
  LimitCircle_t compute_minimum_ring_vertices_to_inner_halfedges_distance(const pmp::SurfaceMesh& mesh,
									  const pmp::Vertex& vertex);

  // minimum distance of a vertex from its connected ring
  LimitCircle_t compute_minimum_vertex_to_ring_distance(const pmp::SurfaceMesh& mesh,
							const pmp::Vertex& vertex);

  // triangle qualities
  std::map<int, double> get_triangle_qualities_map(const pmp::SurfaceMesh& mesh);
  std::set<double>      get_triangle_qualities_set(const pmp::SurfaceMesh& mesh);
}
