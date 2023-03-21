// Sriramajayam

#pragma once

#include <Eigen/Dense>
#include <vector>
#include <array>

namespace vm
{
  Eigen::MatrixXd compute_element_stiffness(const std::vector<std::array<double,2>>& coords);
}
