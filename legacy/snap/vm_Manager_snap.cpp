// Sriramajayam

#include <vm_Manager.h>
#include <vm_inspect.h>
#include <vm_snap.h>
#include <cassert>

namespace vm
{
  // snap vertices to nearby edges
  int Manager::snap_vertices(const double eps_dist_ratio)
  {
    assert(eps_dist_ratio>0. && eps_dist_ratio<1.);
    
    // examine each vertex of each face
    // inspect the closest distance of the vertex to non incident edges
    // if this distance is sufficiently small compared to the smallest edge length of the face at the vertex, snap if possible
    int nsnaps = 0;
    
    // TODO: can restrict the search to just a few faces- perhaps ones with poor quality or concave angles?

    auto face_circulator = mesh.faces();
    for(auto face:face_circulator)
      if(mesh.is_valid(face) && !mesh.is_deleted(face))
	{
	  // does this face need a vertex to be snapped?
	  auto snap_vertices = needs_snap(mesh, face, eps_dist_ratio);

	  if(snap_vertices.empty())
	    continue;
	  
	  // snap a vertex of this face, if possible
	  for(auto& vertex:snap_vertices)
	    {		
	      // halfedge on which to project this vertex
	      auto target_halfedge = closest_halfedge(mesh, face, vertex);
	      
	      // snap, if legal
	      if(is_snap_ok(mesh, vertex, target_halfedge))
		{
		  vm::snap(mesh, curr2ref_vertex_map, vertex, target_halfedge);
		  ++nsnaps;
		  break;
		}
	    }
	}
    
    // done
    return nsnaps;
  }


  
}
