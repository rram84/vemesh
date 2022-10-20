// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // Agglomerate poor quality elements
  // mesh [in, out]       : polygonal mesh with precomputed face qualities
  // q_threshold [in]     : quality threshold to flag modifiable elements
  void merge(pmp::SurfaceMesh& mesh, const pmp::Halfedge& halfedge);
 
}
