// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // examine whether a given vertex needs to be moved
  bool needs_move(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex,
		  const double eps_length_ratio);

  // identify a feasible point to move a vertex
  std::pair<bool, pmp::Point> feasible_move_point(const pmp::SurfaceMesh& mesh,
						  const pmp::Vertex& vertex,
						  const double eps_length_ratio);

  // move a vertex
  void move(pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex, const pmp::Point& X);
}
