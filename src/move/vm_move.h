// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // identify a feasible point to move a vertex
  std::pair<bool, std::pair<double,double>> compute_feasible_vertex_position(const pmp::SurfaceMesh& mesh,
									     const pmp::Vertex& vertex,
									     const double eps_length_ratio,
									     const double eps_degrees,
									     const int num_samples);
  
}
