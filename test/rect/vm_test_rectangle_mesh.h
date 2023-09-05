// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  namespace test
  {
    pmp::SurfaceMesh create_rect_mesh(const double* left_cnr,
				      const double hx, const int nx,
				      const double hy, const int ny);
  }
}
