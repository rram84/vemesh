// Sriramajayam

#include <vm_Manager.h>
#include <vm_vertex_move.h>
#include <iostream>

namespace vm
{
  // moves a vertex
  std::pair<bool, double> Manager::move_vertex(const pmp::Vertex& vertex, const int num_samples, MeshVertexQuality_f qfunc)
  {
    // cannot move boundary vertices
    if(mesh.is_boundary(vertex)==true)
      {
	return {false, -1.0};
      }

    // identify a feasible new position & move
    const auto result = compute_feasible_vertex_position(mesh, vertex, num_samples, qfunc);
        
    // no feasible point
    if(std::get<0>(result)==false)
      {
	return {false, -1.0};
      }

    // found a feasible point
    const auto& update_pos = std::get<1>(result);
    
    // move
    pmp::Point& X = mesh.position(vertex);
    X = update_pos;
    
    // done
    return {true, std::get<2>(result)};
  }


  // try to move all vertices to favorable positions. returns the number of moved vertices
  /*int Manager::move_all_vertices(const int num_samples)
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
    }*/
  
}
