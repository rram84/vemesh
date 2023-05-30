// Sriramajayam

#include <pmp/SurfaceMesh.h>

namespace vm
{
  pmp::SurfaceMesh create_rect_mesh(const double* left_cnr,
				    const double hx, const int nx,
				    const double hy, const int ny);

  // points are assumed to lie in the plane (spatial dimension = 2)
  pmp::SurfaceMesh create_delaunay_triangulation(const std::vector<std::pair<double,double>>& points);

  // delaunay triangulation of a random distribution of points within the unit square
  pmp::SurfaceMesh create_random_delaunay(const double* bot_left_cnr, const double* top_right_cnr, const int num_points);
}
