// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <vector>
#include <utility>

namespace vm
{
  std::vector<std::pair<double,double>> compute_polygon_kernel(const std::vector<pmp::Point>& vertices);
}
