// Sriramajayam

#include <vm_Manager.h>
#include <vm_quality.h>
#include <vm_move.h>

namespace vm
{
  // moves vertices if the length of an incident edge is small
  int Manager::move_vertices(const double eps_len_ratio, const double eps_degrees, const int num_samples)
  {
    int nmoved = 0;

    // identify vertices to be be moved
    // identify a feasible new position
    // move
    auto vert_circulator = mesh.vertices();
    for(auto vertex:vert_circulator)
      if(mesh.is_boundary(vertex)==false)
	{
	  // quality at this vertex
	  auto vert_quality = vertex_quality(mesh, vertex);
	  if(vert_quality.first<eps_len_ratio || vert_quality.second<eps_degrees)
	    {
	      // this vertex needs to be moved. identify a feasible point
	      const std::pair<bool, std::pair<double,double>> feasible_point = compute_feasible_vertex_position(mesh, vertex, eps_len_ratio, eps_degrees, num_samples);
	      if(feasible_point.first==true)
		{
		  // move
		  pmp::Point& X = mesh.position(vertex);
		  X[0] = feasible_point.second.first;
		  X[1] = feasible_point.second.second;
		  ++nmoved;
		}
	    }
	}
    
    // done
    return nmoved;
  }

}
