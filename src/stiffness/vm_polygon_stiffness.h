// Sriramajayam

#pragma once

#include <Eigen/Dense>
#include <pmp/SurfaceMesh.h>

namespace vm
{
  Eigen::MatrixXd compute_polygon_stiffness_matrix(const std::vector<pmp::Point>& coords,
						   const double stabilization = 1.0);
}
