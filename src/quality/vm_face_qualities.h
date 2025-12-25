// Sriramajayam

#pragma once

#include <vm_utils.h>
#include <list>
#include <Eigen/Dense>

namespace vm
{
  namespace quality
  {
    // VEM stiffness matrix of a polygon
    Eigen::MatrixXd vem_stiffness_matrix(const std::vector<pmp::Point>& coords, const double stabilization = 1.0);

    double vem_stability_ratio(const std::vector<pmp::Point>& coords);
    
    // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
    // ratio of the area to the perimeter^2
    double geom_shape(const std::vector<pmp::Point>& coords);

    // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
    // smallest included angle
    double geom_min_angle(const std::vector<pmp::Point>& coords);
  }
}
