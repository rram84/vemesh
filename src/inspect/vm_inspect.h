// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace pmp
{
  // Run checks on a mesh face
  // mesh [in]           : polygon mesh
  // face [in]           : polygon face
  void InspectFace(const pmp::SurfaceMesh& mesh, const pmp::Face& face);
}
