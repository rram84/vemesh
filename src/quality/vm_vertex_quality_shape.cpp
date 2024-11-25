// Sriramajayam

#include <vm_vertex_quality.h>
#include <cmath>

// boost polygon utilities
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace vm {
  
  // boost aliases
  namespace bg             = boost::geometry;
  namespace bgm            = bg::model;
  using boost_point_t      = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_polygon_t    = bgm::polygon<boost_point_t>;

  // measure the quality of a face as the ratio of the area/perimeter^2
  double compute_shape_based_face_quality(const pmp::SurfaceMesh& mesh, const pmp::Face& face) {

    // boost polygon of this face
    boost_polygon_t poly;
    auto v_circulator = mesh.vertices(face);
    for(auto v:v_circulator) {
      const auto& X = mesh.position(v);
      bg::append(poly.outer(), boost_point_t(X[0],X[1]));
    }
    auto first_vertex = *poly.outer().begin();
    bg::append(poly.outer(), first_vertex);

    // area
    double area = bg::area(poly);
    double perim = bg::perimeter(poly);
    return perim*perim/(4.*std::atan(1.)*area);
  }

  
  // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
  // ratio of the area to the perimeter^2
  double compute_shape_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert) {

    // loop over incident faces
    // return the smallest quality among them
    double quality = std::numeric_limits<double>::max();
    auto f_circulator = mesh.faces(vert);
    for(auto f:f_circulator)
      {
  	double min_quality = compute_shape_based_face_quality(mesh, f);
	if(min_quality<quality)
	  quality = min_quality;
      }
    return quality;
  }

}
