// Sriramajayam

#pragma once

#include <vector>
#include <utility>

namespace vm
{
  std::vector<std::pair<double,double>> compute_polygon_sampling(const std::vector<std::pair<double,double>>& vertices, const int num_points);
}

