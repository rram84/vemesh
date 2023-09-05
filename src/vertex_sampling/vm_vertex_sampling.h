// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // random generation of feasible vertex positions
  std::vector<std::pair<double,double>>
    compute_feasible_vertex_positions(const pmp::SurfaceMesh& mesh,
				      const pmp::Vertex&      vertex,
				      const int               num_poly_samples,         // max number of random positions to generate
				      const int               num_edge_samples);       // number of samples to generate per edge  
}
