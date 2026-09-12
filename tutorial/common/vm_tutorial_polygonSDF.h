// Sriramajayam

/** \file vm_tutorial_polygonSDF.h
 * \brief R-tree-accelerated signed distance to one or more closed polygon loops (vm::tutorial::PolygonSDF)
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
     * \brief Signed distance to one or more closed polygon loops
     *
     * Computes the signed distance from a query point to the boundary of a
     * polygon. The sign convention adopted assigns negative distances to points
     * *inside* the polygon and positive to those *outside*.
     * The polygon's boundary is the zero level set of the function. A PolygonSDF is
     * therefore usable directly as a \ref LevelSetFn for \ref adjust_mesh_nodes,
     * \ref clip_mesh and \ref embed_interface, with the polygon playing the
     * role of the negative sub-level set.
     *
     * The boundary may be a single simple polygon or **several disjoint closed
     * loops**, supplied as one ordered vertex list per loop. All loops are
     * indexed into a single R-tree and the even-odd sign test is applied over
     * every loop, so nested loops act as holes (inside outer + inside inner =
     * even = outside) and separate loops as independent components, with no
     * per-loop bookkeeping.
     *
     * Each distance query is evaluated in two parts:
     * - **magnitude:** the Euclidean distance to the nearest boundary segment,
     *   located by a nearest-neighbour query on the R-tree;
     * - **sign:** an inside/outside test that counts crossings of the
     *   \f$+x\f$ horizontal ray from the query point with the boundary (the
     *   even-odd rule), considering only the segments returned by the R-tree.
     *
     * As a result, the per-query cost is \f$O(\log n)\f$
     * in the total number of polygon vertices \f$n\f$.
     *
     * \note Each loop is assumed *simple* (non-self-intersecting) and distinct
     *       loops are assumed not to cross. Each loop is closed and its
     *       orientation corrected internally, so the supplied vertices need not
     *       repeat the first point or follow a particular winding. A single loop
     *       is the ordinary simple-polygon case.
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
       * \brief Construct from a single closed loop's boundary vertices.
       *
       * Convenience overload for the common simple-polygon case; equivalent to
       * passing a single loop to the multi-loop constructor.
       *
       * \param[in] vertices Boundary vertices as flattened \f$x,y\f$ pairs in
       *            order around the polygon, i.e. `{x0, y0, x1, y1, ...}`. At
       *            least three vertices (six entries) are required.
       *
       * \throws std::invalid_argument if fewer than three vertices are supplied.
       */
      PolygonSDF(const std::vector<double>& vertices);

      /**
       * \brief Construct from several closed loops (holes / disjoint pieces).
       *
       * Nested loops act as holes and separate loops as independent components
       * (see the class description); orientation and winding per loop are
       * immaterial.
       *
       * \param[in] loops One flattened \f$x,y\f$ list per closed loop, i.e.
       *            `{{x0,y0,x1,y1,...}, ...}`. Loops with fewer than three
       *            vertices are ignored.
       *
       * \throws std::invalid_argument if no loop has at least three vertices.
       */
      PolygonSDF(const std::vector<std::vector<double>>& loops);
      
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

      // shared construction: loops -> corrected rings -> one segment R-tree
      void build(const std::vector<std::vector<double>>& loops);
      
      rtree_t rtree_;
    };

    /**
     * \brief Read polygon loops from a text file for use with PolygonSDF.
     *
     * One `x y` sample per line, in order around each loop; a blank line
     * separates one closed loop from the next. A file with no blank line is a
     * single loop. Keeps file I/O out of PolygonSDF's construction.
     *
     * \param[in] filename Path to the polygon file.
     * \return One flattened {x0,y0,...} list per loop.
     * \throws std::runtime_error if the file cannot be opened.
     * \ingroup tutorial_utils
     */
    std::vector<std::vector<double>> read_polygon_loops(const std::string& filename);
    
  }
}
