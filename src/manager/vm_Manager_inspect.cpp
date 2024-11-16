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
    auto face_circulator = mesh.faces();
    for(auto face:face_circulator)
      {
	bool flag = inspect_face(mesh, face);
	assert(flag==true);
      }

    assert(mesh.has_face_property("material_id")==true);
    assert(mesh.has_vertex_property("interface_id")==true);
    
    // done
    return;
  }

}
