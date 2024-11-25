// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // Run checks on a mesh face
  // mesh [in]           : polygon mesh
  bool inspect_mesh(const pmp::SurfaceMesh& mesh);

  bool inspect_face(const std::vector<pmp::Point>& coords);

  // inspect correctness of a vertex ring
  bool inspect_vertex_ring(const pmp::SurfaceMesh& mesh,
			   const pmp::Vertex& vertex);
}
