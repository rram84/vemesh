// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <utility>
#include <list>

namespace vm
{
  using MeshVertexQuality_f = std::function<double(const pmp::SurfaceMesh&, const pmp::Vertex&)>;
  
  // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
  // smallest nonzero eigenvalue
  double compute_stiffness_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);

  // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
  // ratio of the area to the perimeter^2
  double compute_shape_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);
}
