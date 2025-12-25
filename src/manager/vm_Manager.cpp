// Sriramajayam

/** \file vm_Manager.cpp
 * \brief Implementation of class vm::Manager
 * \author Ramsharan Rangarajan
 */

#include <vm_Manager.h>
#include <vm_io.h>
#include <filesystem>

namespace vm
{
  // Constructor
  Manager::Manager(const pmp::SurfaceMesh& in_mesh)
    :mesh(in_mesh)
  {
    // sanity checks
    assert(mesh.has_face_property("material_id")==true);
    assert(mesh.has_vertex_property("interface_id")==true);
  }

  // visualize mesh along with face qualities
  void Manager::compute_face_qualities(FaceQuality_f qfunc)
  {
    // "face_quality" property
    if(mesh.has_face_property("face_quality")==false)
      {
	mesh.add_face_property<double>("face_quality");
      }

    // (re)compute
    auto quality = mesh.get_face_property<double>("face_quality");
    auto f_circulator = mesh.faces();
    double q;
    for(auto f:f_circulator)
      {
	q = MeshFaceQuality_f(mesh, f, qfunc);
	quality[f] = q;
      }

    // done
    return;
  }
  
  
  // visualize mesh along with vertex qualities
  void Manager::compute_vertex_qualities(MeshVertexQuality_f qfunc)
  {
    // "vertex_quality" property
    if(mesh.has_vertex_property("vertex_quality")==false)
      {
	mesh.add_vertex_property<double>("vertex_quality");
      }

    // (re)compute
    auto quality = mesh.get_vertex_property<double>("vertex_quality");
    auto v_circulator = mesh.vertices();
    double q;
    for(auto v:v_circulator)
      {
	q = qfunc(mesh, v);
	quality[v] = q;
      }

    // done
    return;
  }

}
