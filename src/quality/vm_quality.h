// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // measure quality of a face as the smallest included angle
  double angle_quality(const pmp::SurfaceMesh& mesh, const pmp::Face& face);
  
  // measure quality of a face as the ratio of smallest and largest edge lengths
  double edge_quality(const pmp::SurfaceMesh& mesh, const pmp::Face& face);
}
