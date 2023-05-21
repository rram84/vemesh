// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <functional>
#include <map>

namespace vm
{
  using LevelSetFunction_t = std::function<double(const double*)>;

  int adjust_mesh_nodes(pmp::SurfaceMesh& mesh, const double phi_eps, const double pert_eps, LevelSetFunction_t& ls_func);
  
  void clip_mesh(pmp::SurfaceMesh& mesh, const double phi_eps, LevelSetFunction_t& lsfunc);

}
