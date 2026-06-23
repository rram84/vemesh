// Sriramajayam

/** \file vm_tutorial_rectangle_mesh.h
 * \brief Structured rectangle/quad mesh generation utility for the tutorials
 * \author Ramsharan Rangarajan
 */

#pragma once

#include <pmp/surface_mesh.h>
#include <array>

/*!
 * \namespace vm::tutorial
 *
 * \brief Helper utilities for tutorial  exampled
 *
 * This namespace contains lightweight mesh-generation and convenience
 * functions that are **not part of the core vm library API**, but are
 * provided to simplify tutorial examples and demonstrate use cases for the library.
 *
 * The utilities in this namespace prioritize simplicity over performance.
 * They make simplifying assumptions and omit extensive error checking.
 *
 */
namespace vm
{
  namespace tutorial
  {
    /**
     * \brief Create a structured rectangular surface mesh.
     *
     * This function generates a planar rectangle mesh over a rectangular
     * domain aligned with the coordinate axes. The mesh lies in the
     * \f$ z = 0 \f$ plane and is intended for tutorial and example use only.
     *
     * The rectangle is defined by its lower-left corner \p left_cnr and a regular
     * grid of vertices with spacings \p hx and \p hy in the x- and y-directions,
     * respectively.
     *
     * A total of \p nx vertices are created along the x-direction and \p ny vertices
     * along the y-direction, resulting in \f$(nx-1)\times(ny-1)\f$ quadrilateral faces.
     *
     * The coordinates of the top-right corner of the rectangle are therefore:
     * \f[
     * (x_{\text{tr}}, y_{\text{tr}}) =
     * \bigl(\text{left\_cnr}[0] + (nx-1)\,hx,\;
     *       \text{left\_cnr}[1] + (ny-1)\,hy\bigr).
     * \f]
     *
     * Vertices are generated row-by-row starting from the lower-left corner,
     * and faces are added as rectangles with consistent orientation.
     *
     * All faces are assigned the same \p domain_id of 0, and all vertices are
     * initialized with \c interface_id = -1.
     *
     *
     * \param[in] left_cnr  Cartesian coordinates \f$(x, y)\f$ of the lower-left corner.
     * \param[in] hx        Grid spacing in the x-direction.
     * \param[in] nx        Number of vertices in the x-direction (must be > 1).
     * \param[in] hy        Grid spacing in the y-direction.
     * \param[in] ny        Number of vertices in the y-direction (must be > 1).
     *
     * \return A pmp::SurfaceMesh representing the rectangular grid.
     * \ingroup tutorial_utils
     */
    pmp::SurfaceMesh create_rectangle_mesh(const std::array<double,2> left_cnr,
					   const double hx, const int nx,
					   const double hy, const int ny);
  }
}
