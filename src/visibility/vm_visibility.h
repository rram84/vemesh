// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  std::vector<std::pair<double,double>>
    compute_visibility_polygon(const pmp::SurfaceMesh& mesh,
			       const pmp::Vertex& vertex);
}
