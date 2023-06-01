// Sriramajayam

#include <pmp/SurfaceMesh.h>

namespace vm
{
  pmp::SurfaceMesh create_rect_mesh(const double* left_cnr,
				    const double hx, const int nx,
				    const double hy, const int ny);

  // points are assumed to lie in the plane (spatial dimension = 2)
  pmp::SurfaceMesh create_delaunay_triangulation(const std::vector<std::pair<double,double>>& points);
}
