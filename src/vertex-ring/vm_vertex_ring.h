// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // compute the vertex ring
  std::vector<pmp::Vertex> get_vertex_ring(const pmp::SurfaceMesh& mesh, const pmp::Vertex& v);

  // inspect if a vertex is connected to a hanging node
  bool is_vertex_connected_to_hanging_node(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex);
    
}
