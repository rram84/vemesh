// Sriramajayam

/** \file vm_face_qualities.h
 * \brief Defines a specific set of face quality metrics
 * \author Ramsharan Rangarajan
 */

#pragma once

#include <vm_utils.h>
#include <list>
#include <Eigen/Dense>

namespace vm
{
  /** \namespace vm::quality
   * \brief Specific set of polygon quality metrics.
   *
   * This namespace collects quality measures for polygonal faces.
   *
   * The library provides three instances of these:
   *
   * - vm::quality::vem_stability_ratio defines the quality of a face using the
   *  the stability ratio of the element stiffness matrix in the VEM.
   *
   * - vm::quality::geom_shape defines the face quality in a geometric sense 
   * in terms of the area and perimeter of the polygon
   * 
   * \sa vm::QualityEvaluator
   */
  namespace quality
  {
    /**
     * \brief Compute the VEM stiffness matrix for a polygon.
     * 
     * The stiffness matrix corresponds to the scalar Poisson problem
     * discretized using the Virtual Element Method (VEM).
     *
     * \param coords Coordinates of polygon vertices (in counterclockwise order).
     * \param stabilization Optional stabilization factor (default = 1.0).
     * \return Stiffness matrix (Eigen::MatrixXd).
     * \sa  vm::vem_stability_ratio
     * \ingroup quality
     */
    Eigen::MatrixXd vem_stiffness_matrix(const std::vector<pmp::Point>& coords,
					 const double stabilization = 1.0);

    /**
     * \brief Compute the stability ratio of a polygon.
     * 
     * The stability ratio is computed as the ratio of the second
     * smallest eigenvalue to the largest eigenvalue of the VEM
     * stiffness matrix. The smallest eigenvalue is expected to be
     * zero (upto numerical precision).
     * 
     * The stiffness matrix corresponds to the scalar Poisson problem
     * discretized using the Virtual Element Method (VEM).
     *
     * A small ratio indicates potential for poor conditioning.
     *
     * \param coords Coordinates of polygon vertices.
     * \return Stability ratio in the range [0,1].
     * \sa vm::vem_stiffness_matrix
     * \ingroup quality
     */
    double vem_stability_ratio(const std::vector<pmp::Point>& coords);

     /**
     * \brief Measure shape quality using area/perimeter^2 ratio.
     * 
     * Returns a normalized measure where 1 indicates an ideal polygon shape.
     * Useful as a geometric measure of quality.
     *
     * \param coords Polygon vertices in counterclockwise order.
     * \return Shape quality metric in the range [0,1].
     */
    double geom_shape(const std::vector<pmp::Point>& coords);
  }
}
