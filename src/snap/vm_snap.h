// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // determine whether the vertex of a face needs to be snapped
  std::vector<pmp::Vertex> needs_snap(pmp::SurfaceMesh& mesh, const pmp::Face& face, const double eps_dist_ratio);
  
  // identify the closest halfedge of a face to a given vertex
  pmp::Halfedge closest_halfedge(pmp::SurfaceMesh& mesh, const pmp::Face& face, const pmp::Vertex& vertex);

  // determine the orthogonal projection of a vertex on a halfedge
  std::pair<bool, pmp::Point> projection_on_halfedge(pmp::SurfaceMesh& mesh,
						     const pmp::Vertex& vertex,
						     const pmp::Halfedge& halfedge);
  
  // examine whether snapping a vertex to its closest point on a half-edge is legal
  bool is_snap_ok(pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex, const pmp::Halfedge& halfedge);
  
  // snap a vertex to its closest point on a halfedge
  void snap(pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex, const pmp::Halfedge& halfedge);
}  

