// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // measure quality of a face as the smallest included angle
  double face_quality(const pmp::SurfaceMesh& mesh, const pmp::Face& face);
  double face_quality(const std::vector<pmp::Point>& coords);
  
  
  // measure quality of a face as the ratio of smallest and largest edge lengths
  double vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);
}
