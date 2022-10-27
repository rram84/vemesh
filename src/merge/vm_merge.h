// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // identify the face along which to merger a given face
  std::pair<bool, pmp::Halfedge> merge_halfedge(pmp::SurfaceMesh& mesh, const pmp::Face& face);
  
  // Agglomerate poor quality elements
  // mesh [in, out]       : polygonal mesh with precomputed face qualities
  // q_threshold [in]     : quality threshold to flag modifiable elements
  void merge(pmp::SurfaceMesh& mesh, const pmp::Halfedge& halfedge);
}
