// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <map>

namespace vm
{
  // determine whether the vertex of a face needs to be snapped
  std::vector<pmp::Vertex> needs_snap(const pmp::SurfaceMesh& mesh, const pmp::Face& face, const double eps_dist_ratio);
  
  // identify the closest halfedge of a face to a given vertex
  pmp::Halfedge closest_halfedge(const pmp::SurfaceMesh& mesh, const pmp::Face& face, const pmp::Vertex& vertex);

  // determine the orthogonal projection of a vertex on a halfedge
  std::pair<bool, pmp::Point> projection_on_halfedge(const pmp::SurfaceMesh& mesh,
						     const pmp::Vertex& vertex,
						     const pmp::Halfedge& halfedge);

  // examine whether snapping a src_vertex to tgt_vertex in a mesh is ok
  bool is_snap_ok(const pmp::SurfaceMesh& mesh, const pmp::Vertex& src_vertex, const pmp::Vertex& tgt_vertex);
  
  // examine whether snapping a vertex to its closest point on a half-edge is legal
  bool is_snap_ok(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex, const pmp::Halfedge& halfedge);
  
  // snap a vertex to its closest point on a halfedge
  void snap(pmp::SurfaceMesh& mesh, std::map<pmp::Vertex, pmp::Vertex>& vertex_map,
	    const pmp::Vertex& vertex, const pmp::Halfedge& halfedge);
}  

