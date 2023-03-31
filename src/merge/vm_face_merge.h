// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <vm_face_quality.h>

namespace vm
{
  // identify the face along which to merger a given face
  std::pair<bool, pmp::Halfedge> find_merge_halfedge(const pmp::SurfaceMesh& mesh, const pmp::Face& face, FaceQuality_f qfunc);
  
  // Agglomerate poor quality elements
  void merge(pmp::SurfaceMesh& mesh, const pmp::Halfedge& halfedge);
}
