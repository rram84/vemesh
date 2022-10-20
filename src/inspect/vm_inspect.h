// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // Run checks on a mesh face
  // mesh [in]           : polygon mesh
  // face [in]           : polygon face
  void inspect_face(pmp::SurfaceMesh& mesh, pmp::Face& face);

  bool inspect_face(const std::vector<pmp::Point>& coords);
}
