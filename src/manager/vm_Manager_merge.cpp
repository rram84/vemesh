// Sriramajayam

#include <vm_Manager.h>
#include <vm_face_merge.h>
#include <iostream>

namespace vm
{
  // merge poor quality elements with neighbors
  std::pair<bool, pmp::Face> Manager::merge_face(const pmp::Face& face, FaceQuality_f qfunc)
  {
    assert(mesh.is_valid(face) && !mesh.is_deleted(face));
    
    // merge along best possible neighbor
    auto result = find_halfedge_for_face_merge(mesh, face, qfunc);
    const auto& success       = result.first;
    const auto& best_halfedge = result.second;
    std::pair<bool, pmp::Face> ret_data{false, face};
    if(success==true)
      {
	ret_data = {success, mesh.face(mesh.opposite_halfedge(best_halfedge))};
	vm::merge_face(mesh, best_halfedge);
      }

    return ret_data;
  }
  
}
