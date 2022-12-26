// Sriramajayam

#include <vm_move.h>
#include <vm_visibility.h>
#include <vm_polygon_sampling.h>
#include <vm_vertex_ring.h>
#include <limits>
#include <set>
#include <iostream>

namespace vm
{
  // identify a feasible point to move a vertex
  std::tuple<bool, std::pair<double,double>, LimitCircle_t> compute_feasible_vertex_position(pmp::SurfaceMesh&  mesh,
											     const pmp::Vertex& vertex,
											     const int          num_samples)
  {
    assert(mesh.is_valid(vertex)==true);
    assert(mesh.is_boundary(vertex)==false);

    // given vertex position
    const pmp::Point given_vertex_pos = mesh.position(vertex);

    // check if "vertex" is connected to a neighbor with valence = 2
    // this case is not currently dealt with
    if(is_vertex_connected_to_hanging_node(mesh, vertex))
      {
	LimitCircle_t lc;
	return {false, {given_vertex_pos[0], given_vertex_pos[1]}, lc};
      }
    
    // compute the visibility polygon
    const std::vector<std::pair<double,double>> vis_poly_verts = compute_visibility_polygon(mesh, vertex);
    
    // get feasible sample points inside the visibility polygon
    const std::vector<std::pair<double,double>> vis_poly_samples = compute_polygon_sampling(vis_poly_verts, num_samples);

    // use the current vertex quality as the datum
    std::pair<double,double> curr_best_pos = {given_vertex_pos[0], given_vertex_pos[1]};
    auto curr_best_quality                 = compute_distance_based_vertex_quality(mesh, vertex);
    
    // examine vertex qualities at the sample points
    pmp::Point& running_vert_pos = mesh.position(vertex);
    bool success = false;
    for(auto& sample:vis_poly_samples)
      {
	// move the vertex to this sample point's location
	running_vert_pos[0] = sample.first;
	running_vert_pos[1] = sample.second;
	
	// evaluate the resulting vertex quality
	auto sample_quality = compute_distance_based_vertex_quality(mesh, vertex);
	
	// Does sample_quality dominate curr_best_quality
	if(sample_quality.radius>curr_best_quality.radius)
	  {
	    curr_best_quality.center[0] = sample_quality.center[0];
	    curr_best_quality.center[1] = sample_quality.center[1];
	    curr_best_quality.radius    = sample_quality.radius;
	    curr_best_pos               = sample;
	    success                     = true;
	  }
      }
    
    // restore the vertex position
    mesh.position(vertex) = given_vertex_pos;

    // done
    return {success, curr_best_pos, curr_best_quality};
  }

}
