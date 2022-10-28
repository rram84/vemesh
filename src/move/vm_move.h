// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // examine whether a given vertex needs to be moved
  bool needs_move(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex,
		  const double eps_length_ratio);
}
