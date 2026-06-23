// Sriramajayam

/** \file vm_tutorial_polygonSDF.h
 * \brief R-tree-accelerated signed distance to a simple polygon (vm::tutorial::PolygonSDF)
 * \author Ramsharan Rangarajan
 */

#pragma once

#include <vm_utils.h>

#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/index/rtree.hpp>

#include <string>
#include <vector>

namespace vm
{
  namespace tutorial
  {
    /**
     * \brief Signed distance to a simple polygon
     *
     * Computes the signed distance from a query point to the boundary of a
     * simple polygon. The sign convention adopted assigns negative distances to points
     * *inside* the polygon and positive to those *outside*. 
     * The polygon's boundary is the zero level set of the function. A PolygonSDF is
     * therefore usable directly as a \ref LevelSetFn for \ref adjust_mesh_nodes,
     * \ref clip_mesh and \ref embed_interface, with the polygon playing the
     * role of the negative sub-level set.
     *
     * The polygon is supplied as an ordered list of vertices.
     * The implementation here builds an R-tree of the boundary segments for fast queries.
     * 
     * Each distance query is evaluated in two parts:
     * - **magnitude:** the Euclidean distance to the nearest boundary segment,
     *   located by a nearest-neighbour query on the R-tree;
     * - **sign:** an inside/outside test that counts crossings of the
     *   \f$+x\f$ horizontal ray from the query point with the boundary (the
     *   even-odd rule), considering only the segments returned by the R-tree.
     *
     * As a result, the per-query cost is \f$O(\log n)\f$ 
     * in the number of polygon vertices \f$n\f$.
     *
     * \note The polygon is assumed to be *simple* (non-self-intersecting). Its
     *       boundary is closed and its orientation corrected internally, so the
     *       supplied vertices need not repeat the first point or follow a
     *       particular winding.
     *
     * \see LevelSetFn
     * \see adjust_mesh_nodes
     * \see embed_interface
     *
     * \ingroup tutorial_utils
     */
    class PolygonSDF
    {
    public:
      /**
       * \brief Construct from polygon boundary vertices.
       *
       * \param[in] vertices Boundary vertices as flattened \f$x,y\f$ pairs in
       *            order around the polygon, i.e. `{x0, y0, x1, y1, ...}`. At
       *            least three vertices (six entries) are required.
       *
       * \throws std::invalid_argument if fewer than three vertices are supplied.
       */
      PolygonSDF(const std::vector<double>& vertices);

      /**
       * \brief Construct from a text file of boundary samples.
       *
       * \param[in] filename Path to a whitespace-separated file with one
       *            `x y` boundary sample per line, in order around the polygon.
       *
       * \throws std::runtime_error if the file cannot be opened.
       * \throws std::invalid_argument if fewer than three vertices are read.
       */
      PolygonSDF(const std::string& filename);

      /**
       * \brief Signed distance at a point.
       *
       * \param[in] x x-coordinate of the query point.
       * \param[in] y y-coordinate of the query point.
       *
       * \return Signed distance to the polygon boundary: negative inside,
       *         positive outside, zero on the boundary.
       */
      double operator()(double x, double y) const;

      /**
       * \brief Signed distance at a point, matching the \ref LevelSetFn signature.
       *
       * \param[in] X Pointer to the two coordinates \f$(x,y)\f$ of the query point.
       *
       * \return Signed distance to the polygon boundary (see
       *         \ref operator()(double,double) const).
       */
      double operator()(const double* X) const { return (*this)(X[0], X[1]); }

    private:
      using segment_t = vm::bgm::segment<vm::boost_point_t>;
      using rtree_t   = boost::geometry::index::rtree<segment_t, boost::geometry::index::rstar<16>>;

      // shared construction: vertices -> corrected polygon -> segment R-tree
      void build(const std::vector<double>& vertices);

      rtree_t rtree_;
    };

  }
}
