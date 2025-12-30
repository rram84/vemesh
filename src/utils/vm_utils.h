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

}
