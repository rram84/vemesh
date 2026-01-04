// Sriramajayam

/** \file vm_utils.h
 * \brief Defines Boost.Geometry and pmp::SurfaceMesh related utility functions
 * \author Ramsharan Rangarajan
 */

#pragma once

#include <pmp/surface_mesh.h>
#include <functional>

// boost polygon utilities
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace vm
{
  /** \namespace bg
   * \brief Alias for the Boost.Geometry namespace.
   * \ingroup utils
   */
  namespace bg  = boost::geometry;

  /** \namespace bgm
   * \brief Alias for Boost.Geometry model types.
   * \ingroup utils
   */
  namespace bgm = bg::model;

  /** \brief 2D Cartesian point type.
   *
   * Represents a point in two-dimensional Cartesian space using double-precision coordinates.
   * \ingroup utils
   */
  using boost_point_t = bgm::point<double, 2, bg::cs::cartesian>;

  /** \brief Simple 2D polygon with counterclockwise orientation.
   *
   * Represents a simple polygon whose outer boundary is stored
   * in counterclockwise (CCW) order. This convention is required
   * for robust Boolean and topological operations in Boost.Geometry.
   * \ingroup utils
   */
  using boost_polygon_t        = bgm::polygon<boost_point_t, false>; // false = ccw orientation

  /** \brief Collection of polygons.
   *
   * Represents a set of polygons, typically produced as the result
   * of Boolean operations such as union, intersection, or difference.
   * \ingroup utils
   */
  using boost_multi_polygon_t  = bgm::multi_polygon<boost_polygon_t>;

  /** \brief Axis-aligned bounding box in 2D.
   *
   * Represents a rectangular bounding box aligned with the
   * Cartesian coordinate axes, defined by two corner points.
   * \ingroup utils
   */
  using boost_box_t        = bgm::box<boost_point_t>;

  /** \brief 2D polyline (line string).
   *
   * Represents an ordered sequence of points forming a polyline.
   * Used for representing edges
   * \ingroup utils
   */
  using boost_linestring_t = bgm::linestring<boost_point_t>;

  /** \brief Create a 2D Boost.Geometry polygon from a list of points.
   *
   * Constructs a polygon by interpreting the input points as an
   * ordered boundary loop in the plane. The polygon is represented in
   * Boost.Geometry format and is explicitly closed by repeating the
   * first vertex at the end.
   *
   * \param[in] coords Ordered list of polygon vertices.
   *                  Each point is interpreted using its (x,y) components.
   *                  Should have length at least 3.
   *
   * \return A Boost.Geometry polygon whose outer ring corresponds to
   *         the input vertex sequence.
   *
   * \note Does not check is the resulting polygon is valid, simple, or correctly oriented.
   * \ingroup utils
   */
  boost_polygon_t make_polygon(const std::vector<pmp::Point>& coords);

  /** \brief Helper to compute the 1-ring vertices around a given interior vertex in a mesh.
   *
   * The vertex ring consists of the set of vertices connected to the given vertex
   * via the mesh faces.
   *
   * For each face incident to the vertex, the vertex list is
   * rotated so that the target vertex is first, then all other vertices except the
   * first two (including the target) are appended to the ring. Non-manifold edges
   * are removed during this process.
   *
   * \param[in] v The vertex whose ring is to be computed. Should *not* lie on the boundary of the mesh
   * \param[in] mesh The mesh							     
   * \return A vector of vertices forming the vertex ring around `v`.
   *
   * \note The order of vertices in the returned vector depends on the order of
   *       faces incident to `v` in the mesh.
   * \note Non-manifold edges are removed to avoid duplicate or invalid connections.
   * \ingroup utils
   */
  std::vector<pmp::Vertex> get_environment_vertices(const pmp::Vertex& v,
						    const pmp::SurfaceMesh& mesh);

  /**
   * \brief Compute the ordered one-ring vertex environment of an interior mesh vertex.
   *
   * Returns the cyclically ordered list of vertices of the polygon 
   * that is the union of faces incident at a vertex.
   *
   * \param[in] v    An interior vertex of the mesh.
   * \param[in] mesh The surface mesh containing \p v.
   *
   * \return An ordered list of vertices forming the one-ring
   *         environment of \p v.
   * \ingroup utils
   */
  boost_polygon_t get_environment_polygon(const pmp::Vertex& v,
					  const pmp::SurfaceMesh& mesh);

  /**
   * \brief Retrieve the list of vertices directly connected to a given vertex.
   *
   * Returns the coordinates of all vertices that form an edge with the vertex.
   *
   * \param[in] v    A vertex of the mesh.
   * \param[in] mesh The surface mesh containing \p v.
   *
   * \return A list of points corresponding to vertices directly
   *         connected to \p v by an edge in the mesh.
   *
   * \note The order of the returned vertices follows the order of the
   *       outgoing halfedge circulator provided by the mesh.
   *
   * \ingroup utils
   */
  std::vector<pmp::Point> get_connected_vertices(const pmp::Vertex &v,
						 const pmp::SurfaceMesh& mesh);
  
}
