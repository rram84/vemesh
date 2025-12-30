// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <functional>

// boost polygon utilities
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace vm
{
  // boost aliases
  namespace bg  = boost::geometry;
  namespace bgm = bg::model;
  using boost_point_t          = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_polygon_t        = bgm::polygon<boost_point_t, false>; // false = ccw orientation
  using boost_multi_polygon_t  = bgm::multi_polygon<boost_polygon_t>;
  using boost_box_t        = bgm::box<boost_point_t>;
  using boost_linestring_t = bgm::linestring<boost_point_t>;

  // create a boost polygon from a set of vertices
  boost_polygon_t make_polygon(const std::vector<pmp::Point>& coords);

  //! \brief Helper to compute the 1-ring vertices around a given interior vertex in a mesh.
    //!
    //! The vertex ring consists of the set of vertices connected to the given vertex
    //! via the mesh faces.
    //!
    //! For each face incident to the vertex, the vertex list is
    //! rotated so that the target vertex is first, then all other vertices except the
    //! first two (including the target) are appended to the ring. Non-manifold edges
    //! are removed during this process.
    //!
    //! \param[in] v The vertex whose ring is to be computed. Should *not* lie on the boundary of the mesh
    //! \param[in] mesh The mesh							     
    //! \return A vector of vertices forming the vertex ring around `v`.
    //!
    //! \note The order of vertices in the returned vector depends on the order of
    //!       faces incident to `v` in the mesh.
    //! \note Non-manifold edges are removed to avoid duplicate or invalid connections.
  std::vector<pmp::Vertex> get_environment_vertices(const pmp::Vertex& v, const pmp::SurfaceMesh& mesh);

  boost_polygon_t make_environment_polygon(const pmp::Vertex& v, const pmp::SurfaceMesh& mesh);
  
  std::vector<pmp::Point> get_connected_vertices(const pmp::Vertex &v, const pmp::SurfaceMesh& mesh);
  


}
