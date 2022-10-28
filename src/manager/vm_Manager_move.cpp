// Sriramajayam

#include <vm_Manager.h>
#include <vm_move.h>

namespace vm
{
  // moves vertices if the length of an incident edge is small
  int Manager::move(const double eps_len_ratio)
  {
    int nmoved = 0;

    // identify vertices to be be moved
    // identify a feasible new position
    // move
    auto vert_circulator = mesh.vertices();
    for(auto vertex:vert_circulator)
      if(mesh.is_boundary(vertex)==false)
	if(needs_move(mesh, vertex, eps_len_ratio))
	  {
	    const auto result = feasible_move_point(mesh, vertex, eps_len_ratio);
	    const auto& success = result.first;
	    const auto& new_pt  = result.second;
	    if(success==true)
	      {
		vm::move(mesh, vertex, new_pt);
		++nmoved;
	      }
	  }

    // done
    return nmoved;
  }

}
