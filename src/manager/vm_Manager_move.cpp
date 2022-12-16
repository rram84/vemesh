// Sriramajayam

#include <vm_Manager.h>
#include <vm_quality.h>
#include <vm_move.h>

namespace vm
{
  // compute the average edge length emanating from a vertex
  double compute_average_edge_length_at_vertex(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex)
  {
    double hsum   = 0.;
    int    hcount = 0;
    
    auto h_circulator = mesh.halfedges(vertex);
    const auto& X     = mesh.position(vertex);
    for(auto h:h_circulator)
      {
	const auto& Y = mesh.position(mesh.to_vertex(h));
	hsum += std::sqrt((X[0]-Y[0])*(X[0]-Y[0])+(X[1]-Y[1])*(X[1]-Y[1]));
	++hcount;
      }
    return hsum/static_cast<double>(hcount);
  }

  
  // moves vertices if the length of an incident edge is small
  std::pair<int,int> Manager::move_vertices(const double eps_len_ratio, const int num_samples)
  {
    int num_vertices_needs_move = 0;
    int num_vertices_moved      = 0;

    // identify vertices to be be moved.
    // identify a feasible new position
    // move
    auto vert_circulator = mesh.vertices();
    for(auto vertex:vert_circulator)
      if(mesh.is_boundary(vertex)==false)
	{
	  // average edge length at this vertex
	  const double havg = compute_average_edge_length_at_vertex(mesh, vertex);
								    
	  // distance-based quality at this vertex
	  auto vertex_quality = compute_distance_based_vertex_quality(mesh, vertex);

	  // needs to be moved?
	  if(vertex_quality/havg < eps_len_ratio)
	    {
	      ++num_vertices_needs_move;
	      
	      // this vertex needs to be moved. identify a feasible point
	      const std::pair<bool, std::pair<double,double>> feasible_point = compute_feasible_vertex_position(mesh, vertex, num_samples);
	      if(feasible_point.first==true)
		{
		  // move
		  pmp::Point& X = mesh.position(vertex);
		  X[0] = feasible_point.second.first;
		  X[1] = feasible_point.second.second;
		  ++num_vertices_moved;
		}
	    }
	}
    
    // done
    return {num_vertices_moved, num_vertices_needs_move-num_vertices_moved};
  }

}
