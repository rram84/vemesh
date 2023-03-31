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


  // try to move all vertices to favorable positions. returns the number of moved vertices
  int Manager::move_all_vertices(const int num_samples)
  {
    // list of vertex qualities
    std::list<std::pair<pmp::Vertex, double>> vlist{};
    auto v_circulator = mesh.vertices();
    for(auto v:v_circulator)
      if(mesh.is_boundary(v)==false)
	{
	  auto lc = compute_distance_based_vertex_quality(mesh, v);
	  vlist.push_back({v,lc.radius});
	}

    // sort the list of vertices in increasing order of quality
    vlist.sort( [](const auto& a, const auto& b){ return a.second<b.second; } );

    // attempt to relax vertices in order
    int nmoved = 0;
    for(auto& it:vlist)
      {
	const auto& vertex = it.first;
	const auto result  = compute_feasible_vertex_position(mesh, vertex, num_samples);
	if(std::get<0>(result)==true)
	  {
	    ++nmoved;

	    // update coordinates
	    pmp::Point& X = mesh.position(vertex);
	    const auto& Y = std::get<1>(result);
	    X[0] = Y.first;
	    X[1] = Y.second;
	  }
      }
    
    return nmoved;
  }
  
}
