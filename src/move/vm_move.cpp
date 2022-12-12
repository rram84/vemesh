// Sriramajayam

#include <vm_move.h>
#include <vm_visibility.h>
#include <vm_polygon_sampling.h>
#include <vm_quality.h>
#include <vm_inspect.h>
#include <limits>
#include <set>

namespace vm
{
  bool is_vertex_connected_to_hanging_node(const pmp::SurfaceMesh& mesh,
					   const pmp::Vertex&      vertex)
  {
    // the one-ring of vertices should not have any repetitions
    const std::vector<pmp::Vertex> vertex_ring = get_vertex_ring(mesh, vertex);
    const int nRingVerts = static_cast<int>(vertex_ring.size());
    std::set<int> vertex_set{};
    for(auto& v:vertex_ring)
      vertex_set.insert(v.idx());

    // true if there is a repetition, false otherwise
    return (static_cast<int>(vertex_set.size())!=nRingVerts);
  }

  
  // identify a feasible point to move a vertex
  std::pair<bool, std::pair<double,double>> compute_feasible_vertex_position(pmp::SurfaceMesh& mesh,
									     const pmp::Vertex&      vertex,
									     const double            eps_length_ratio,
									     const double            eps_degrees,
									     const int               num_samples)
  {
    assert(mesh.is_valid(vertex)==true);
    assert(mesh.is_boundary(vertex)==false);

    // given vertex position
    const pmp::Point given_vertex_pos = mesh.position(vertex);
    
    // check if "vertex" is connected to a neighbor with valence = 2
    // this case is not currently dealt with
    if(is_vertex_connected_to_hanging_node(mesh, vertex))
      return {false, {given_vertex_pos[0], given_vertex_pos[1]}};
    
    // compute the visibility polygon
    const std::vector<std::pair<double,double>> vis_poly_verts = compute_visibility_polygon(mesh, vertex);
    
    // get feasible sample points inside the visibility polygon
    const std::vector<std::pair<double,double>> vis_poly_samples = compute_polygon_sampling(vis_poly_verts, num_samples);

    // use the current vertex quality as the datum
    std::pair<double,double> curr_best_pos     = {given_vertex_pos[0], given_vertex_pos[1]};
    std::pair<double,double> curr_best_quality = vertex_quality(mesh, vertex);
    
    // examine vertex qualities at the sample points
    pmp::Point& running_vert_pos = mesh.position(vertex);
    bool success = false;
    for(auto& sample:vis_poly_samples)
      {
	// move the vertex to this sample point's location
	running_vert_pos[0] = sample.first;
	running_vert_pos[1] = sample.second;
	
	// evaluate the resulting vertex quality
	const std::pair<double,double> sample_quality = vertex_quality(mesh, vertex);

	// this vertex is better:
	// (i)  if it improves both metrics
	// (ii) otherwise
	//    (a) improves edge length that is below tolerance, while keeping angle above tolerance
	//    (b) improves angle that is below tolerance,       while keeping edge length above tolerance
	if( (sample_quality.first>curr_best_quality.first && sample_quality.second>curr_best_quality.second) || 	               
	    (curr_best_quality.first<eps_length_ratio     && sample_quality.first>curr_best_quality.first   && sample_quality.second>eps_degrees) ||
	    (curr_best_quality.second<eps_degrees         && sample_quality.second>curr_best_quality.second && sample_quality.first>eps_length_ratio))
	  {
	    curr_best_quality = sample_quality;
	    curr_best_pos     = sample;
	    success           = true;
	  }
      }

    // restore the vertex position
    mesh.position(vertex) = given_vertex_pos;

    // done
    return {success, curr_best_pos};
  }

}
