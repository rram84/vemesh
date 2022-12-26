// Sriramajayam

#include <vm_Manager.h>
#include <vm_move.h>
#include <iostream>

namespace vm
{
  // moves a vertex
  std::pair<bool, LimitCircle_t> Manager::move_vertex(const pmp::Vertex& vertex, const int num_samples)
  {
    // cannot move boundary vertices
    if(mesh.is_boundary(vertex)==true)
      {
	LimitCircle_t lc;
	return {false, lc};
      }

    
    // identify a feasible new position & move
    const auto result = compute_feasible_vertex_position(mesh, vertex, num_samples);

    // no feasible point
    if(std::get<0>(result)==false)
      {
	LimitCircle_t lc;
	return {false, lc};
      }

    // found a feasible point
    const auto& update_pos = std::get<1>(result);
    
    // move
    pmp::Point& X = mesh.position(vertex);
    X[0] = update_pos.first;
    X[1] = update_pos.second;
    const auto& lc = std::get<2>(result);

    // done
    return {true, lc};
  }

}
