// Sriramajayam

#include <vm_Manager.h>
#include <vm_inspect.h>
#include <set>

namespace vm
{
  // inspect validity of the mesh
  void Manager::inspect_mesh() const
  {
    assert(mesh.n_vertices()>0);
    assert(mesh.n_faces()>0);
    assert(mesh.n_edges()>0);

    // inspect faces
    bool flag = true;
    auto face_circulator = mesh.faces();
    for(auto face:face_circulator)
      {
	bool f_flag = inspect_face(mesh, face);
	if(f_flag==false)
	  flag = false;
      }

    assert(mesh.has_face_property("material_id")==true);
    assert(mesh.has_vertex_property("interface_id")==true);
    assert(flag==true && "Mesh is invalid");

    // done
    return;
  }

}
