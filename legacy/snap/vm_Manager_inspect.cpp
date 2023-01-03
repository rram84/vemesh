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
      inspect_face(mesh, face);

    // inverse vertex map should:
    // (i)  refer only to old vertices
    // (ii) when the old vertex is deleted & new vertex is not
    const int nvertices = mesh.n_vertices();
    for(auto& it:curr2ref_vertex_map)
      {
	const auto& new_v = it.first;
	const auto& old_v = it.second;
	assert(old_v.idx()>=0 && old_v.idx()<nvertices);
	assert(new_v.idx()>=nvertices);
	assert(mesh.is_deleted(old_v)==true && mesh.is_deleted(new_v)==false);
      }

    // inverse map should be 1-1
    std::set<int> vert_indices{};
    auto vert_circulator = mesh.vertices();
    for(auto v:vert_circulator)
      {
	if(v.idx()<nvertices) // old vertex retained
	  {
	    assert(curr2ref_vertex_map.find(v)==curr2ref_vertex_map.end());
	    vert_indices.insert(v.idx());
	  }
	else                 // new vertex created
	  {
	    auto it = curr2ref_vertex_map.find(v);
	    assert(it!=curr2ref_vertex_map.end());
	    vert_indices.insert(it->second.idx());
	  }
      }
    assert(static_cast<int>(vert_indices.size())==nvertices);
    int count = 0;
    for(auto& it:vert_indices)
      {
	assert(it==count);
	++count;
      }
      
    // done
    return;
  }

}
