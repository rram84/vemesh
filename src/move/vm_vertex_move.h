// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <vm_vertex_quality.h>
#include <tuple>

namespace vm
{
  // identify a feasible point to move a vertex
  std::tuple<bool, pmp::Point, double> compute_improved_vertex_position(pmp::SurfaceMesh          &mesh,
									const pmp::Vertex         &vertex,
									const int                 num_poly_samples,
									const int                 num_edge_samples,
									const MeshVertexQuality_f qfunc);
  // random generation of feasible vertex positions
  std::vector<std::pair<double,double>>
    compute_feasible_vertex_positions(const pmp::SurfaceMesh& mesh,
				      const pmp::Vertex&      vertex,
				      const int               num_poly_samples,         // max number of random positions to generate
				      const int               num_edge_samples);       // number of samples to generate per edge  
}
