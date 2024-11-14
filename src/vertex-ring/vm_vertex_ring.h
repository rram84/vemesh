// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // compute the vertex ring
  std::vector<pmp::Vertex> get_vertex_ring(const pmp::SurfaceMesh& mesh, const pmp::Vertex& v);


  // compute the average edge length emanating from a vertex
  double compute_average_edge_length_at_vertex(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex);
    
}
