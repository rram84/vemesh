// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <utility>
#include <list>

namespace vm
{
  using MeshVertexQuality_f = std::function<double(const pmp::SurfaceMesh&, const pmp::Vertex&)>;
  
  // measure quality as the ratio of the distance of a vertex to its enclosing linestring to the longest halfedge
  // defined only for non-boundary vertices, not connected to hanging nodes
  double compute_distance_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);

  // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
  // smallest nonzero eigenvalue
  double compute_stiffness_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);

  // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
  // ratio of the area to the perimeter^2
  double compute_shape_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);
  
  struct LimitCircle_t
  {
    double center[2];
    double radius; 
  };
  
  // measure quality as the ratio of the distance of a vertex to its enclosing linestring to the longest halfedge
  // defined only for non-boundary vertices, not connected to hanging nodes
  LimitCircle_t compute_limit_circle_for_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);

  // compute the minimum distance of ring vertices to the edges incident at a vertex
  LimitCircle_t compute_minimum_ring_vertices_to_inner_halfedges_distance(const pmp::SurfaceMesh& mesh,
									  const pmp::Vertex& vertex);

  // minimum distance of a vertex from its connected ring
  LimitCircle_t compute_minimum_vertex_to_ring_distance(const pmp::SurfaceMesh& mesh,
							const pmp::Vertex& vertex);

}
