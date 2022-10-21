// Sriramajayam

#include <vm_manager.h>
#include <vm_inspect.h>
#include <cassert>

namespace vm
{
  // snap vertices to nearby edges
  int Manager::snap(const double eps_dist_ratio)
  {
    assert(eps_dist_ratio>0. && eps_dist_ratio<1.);
    
    // examine each vertex of each face
    // inspect the closest distance of the vertex to non incident edges
    // if this distance is sufficiently small compared to the smallest edge length of the face at the vertex, snap if possible
    int nsnaps = 0;
    
    // TODO: can restrict the search to just a few faces- perhaps ones with poor quality or concave angles?

    auto face_circulator = mesh.faces();
    for(auto face:faces)
      if(mesh.is_valid(face) && !mesh.is_deleted(face))
	{
	  auto snap_vertices = needs_snap(mesh, face, eps_dist_ratio);                 // does this face need a vertex to be snapped?
	  if(!snap_vertcies.empty())
	    {
	      // do something
	    }
	}
    
    // done
    return nsnaps;
  }


  
}
