// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // determine whether the vertex of a face needs to be snapped
  std::vector<pmp::Vertex> needs_snap(pmp::SurfaceMesh& mesh, const pmp::Face& face,
				      const double eps_dist_ratio);
  
  // identify the closest halfedge of a face to a given vertex
  pmp::Halfedge closest_halfedge(pmp::SurfaceMesh& mesh, const pmp::Face& face, const pmp::Vertex& vertex);
  
}  

