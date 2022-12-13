// Sriramajayam

#include <vm_move.h>
#include <vm_visibility.h>
#include <vm_polygon_sampling.h>
#include <vm_vertex_ring.h>
#include <vm_quality.h>
#include <limits>
#include <set>

namespace vm
{
  // decide quality dominance
  struct QualityDominance
  {
    double eps_length_ratio;
    double eps_degrees;
    double norm_angle;
    
    QualityDominance(const double eps_length_ratio_val, const double eps_degrees_val)
      :eps_length_ratio(eps_length_ratio_val),
       eps_degrees(eps_degrees_val)
    {}

    // determine if quality A dominates quality B
    bool operator()(const std::pair<double,double>& A,
		    const std::pair<double,double>& B)
    {
      // A better than B in both metrics
      if(A.first>B.first && A.second>B.second)
	return true;
      
      // A lies outside the forbidden zone while B lies within
      if( (A.first>eps_length_ratio && A.second>eps_degrees) && (B.first<eps_length_ratio || B.second<eps_degrees))
	return true;
      
      // Otherwise, assume B dominates A
      return false;
    }
  };

  
  // identify a feasible point to move a vertex
  std::pair<bool, std::pair<double,double>> compute_feasible_vertex_position(pmp::SurfaceMesh&  mesh,
									     const pmp::Vertex& vertex,
									     const double       eps_length_ratio,
									     const double       eps_degrees,
									     const int          num_samples)
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
    std::pair<double,double> curr_best_quality = {compute_distance_based_vertex_quality(mesh, vertex), compute_angle_based_vertex_quality(mesh, vertex)};

    // Quality dominance
    QualityDominance QD(eps_length_ratio, eps_degrees);
    
    // examine vertex qualities at the sample points
    pmp::Point& running_vert_pos = mesh.position(vertex);
    bool success = false;
    for(auto& sample:vis_poly_samples)
      {
	// move the vertex to this sample point's location
	running_vert_pos[0] = sample.first;
	running_vert_pos[1] = sample.second;
	
	// evaluate the resulting vertex quality
	std::pair<double,double> sample_quality = {compute_distance_based_vertex_quality(mesh, vertex), compute_angle_based_vertex_quality(mesh, vertex)};
	
	// Does sample_quality dominate curr_best_quality
	if( QD(sample_quality, curr_best_quality)==true )
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
