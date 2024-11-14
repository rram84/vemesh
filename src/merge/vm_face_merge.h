// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <vm_face_quality.h>

namespace vm
{
  // identify the face along which to merger a given face
  std::tuple<bool, double, pmp::Halfedge> find_halfedge_for_face_merge(const pmp::SurfaceMesh& mesh, const pmp::Face& face, FaceQuality_f qfunc);
  
  // Agglomerate poor quality elements
  void merge_face(pmp::SurfaceMesh& mesh, const pmp::Halfedge& halfedge);
}
