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
  using boost_polygon_t        = bgm::polygon<boost_point_t, false>;
  using boost_multi_polygon_t  = bgm::multi_polygon<boost_polygon_t>;
  using boost_box_t        = bgm::box<boost_point_t>;
  using boost_linestring_t = bgm::linestring<boost_point_t>;

  using FaceQuality_f     = std::function<double(const std::vector<pmp::Point>&)>;
  using MeshFaceQuality_f = std::function<double(const pmp::SurfaceMesh&, const pmp::Face&)>;
  using MeshVertexQuality_f = std::function<double(const pmp::SurfaceMesh&, const pmp::Vertex&)>;
  
  // compute the vertex ring
  std::vector<pmp::Vertex> get_vertex_ring(const pmp::SurfaceMesh& mesh, const pmp::Vertex& v);

  // Run checks on a mesh face
  // mesh [in]           : polygon mesh
  bool inspect_mesh(const pmp::SurfaceMesh& mesh);

  bool inspect_face(const std::vector<pmp::Point>& coords);
}
