// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <functional>
#include <map>

namespace vm
{
  using LevelSetFunction_t = std::function<double(const double*)>;
  
  void clip_tri_mesh(pmp::SurfaceMesh& mesh, LevelSetFunction_t& lsfunc);

  void clip_quad_mesh(pmp::SurfaceMesh& mesh, LevelSetFunction_t& lsfunc);
  
  void clip_mesh_prep(pmp::SurfaceMesh& mesh, const int num_verts_per_face,
		      LevelSetFunction_t& lsfunc,
		      std::map<pmp::Edge, pmp::Vertex>& edge2vertexMap,
		      std::map<pmp::Face, std::vector<int>>& cutfaces2invertsMap);
  
  pmp::SurfaceMesh renumber_mesh_vertices(const pmp::SurfaceMesh& mesh);
}
