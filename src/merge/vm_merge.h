// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // identify the face along which to merger a given face
  std::pair<bool, pmp::Halfedge> merge_halfedge(const pmp::SurfaceMesh& mesh, const pmp::Face& face);
  
  // Agglomerate poor quality elements
  void merge(pmp::SurfaceMesh& mesh, const pmp::Halfedge& halfedge);
}
