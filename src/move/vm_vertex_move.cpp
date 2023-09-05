// Sriramajayam

#include <vm_vertex_move.h>
#include <vm_vertex_sampling.h>
#include <limits>
#include <set>
#include <iostream>

namespace vm
{
  // identify a feasible point to move a vertex
  std::tuple<bool, pmp::Point, double> compute_improved_vertex_position(pmp::SurfaceMesh          &mesh,
									const pmp::Vertex         &vertex,
									const int                 num_poly_samples,
									const int                 num_edge_samples,
									const MeshVertexQuality_f qfunc)
  {
    assert(mesh.is_valid(vertex)==true);
    assert(mesh.is_boundary(vertex)==false);

    // given vertex position
    const pmp::Point given_vertex_pos = mesh.position(vertex);

    // get feasible sample points inside the visibility polygon
    const std::vector<std::pair<double,double>> feasible_samples = compute_feasible_vertex_positions(mesh, vertex, num_poly_samples, num_edge_samples);
    
    // use the current vertex quality as the datum
    std::pair<double,double> curr_best_pos = {given_vertex_pos[0], given_vertex_pos[1]};
    double curr_best_quality = qfunc(mesh, vertex);
    
    // examine vertex qualities at the sample points
    pmp::Point& running_vert_pos = mesh.position(vertex);
    bool success = false;
    for(auto& sample:feasible_samples)
      {
	// move the vertex to this sample point's location
	running_vert_pos[0] = sample.first;
	running_vert_pos[1] = sample.second;
	
	// evaluate the resulting vertex quality
	double sample_quality = qfunc(mesh, vertex);
	
	// Does sample_quality dominate curr_best_quality
	if(sample_quality>curr_best_quality)
	  {
	    curr_best_quality = sample_quality;
	    curr_best_pos               = sample;
	    success                     = true;
	  }
      }
    
    // restore the vertex position
    mesh.position(vertex) = given_vertex_pos;

    // done
    return {success, pmp::Point(curr_best_pos.first,curr_best_pos.second,given_vertex_pos[2]), curr_best_quality};
  }

}
