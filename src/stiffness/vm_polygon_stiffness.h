// Sriramajayam

#pragma once

#include <Eigen/Dense>
#include <vector>
#include <array>

namespace vm
{
  Eigen::MatrixXd compute_polygon_stiffness_matrix(const std::vector<std::array<double,2>>& coords,
						   const double stabilization = 1.0);
}
