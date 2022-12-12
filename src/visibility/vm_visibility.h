// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
   // compute the vertex ring
  std::vector<pmp::Vertex> get_vertex_ring(const pmp::SurfaceMesh& mesh, const pmp::Vertex& v);

  // compute the visibility polygon
  std::vector<std::pair<double,double>>
    compute_visibility_polygon(const pmp::SurfaceMesh& mesh,
			       const pmp::Vertex& vertex);
}
