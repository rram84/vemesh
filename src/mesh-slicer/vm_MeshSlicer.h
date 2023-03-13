// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <functional>

namespace vm
{
  using LevelSetFunction_t = std::function<double(const double*)>;
  
  void clip_tri_mesh(pmp::SurfaceMesh& mesh, LevelSetFunction_t& lsfunc);
}
