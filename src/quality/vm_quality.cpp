// Sriramajayam

#include <vm_quality.h>
#include <cmath>
#include <cassert>

namespace vm
{
  // measure quality of a face as the smallest included angle
  double face_quality(const pmp::SurfaceMesh& mesh, const pmp::Face& face)
  {
    const int nedges = mesh.valence(face);
    std::vector<double> angles(nedges);
    
    auto h_ab = mesh.halfedge(face);
    for(int count=0; count<nedges; ++count)
      {
	// vertices of successive edges
	const auto vert_a = mesh.from_vertex(h_ab);
	const auto vert_b = mesh.to_vertex(h_ab);
	auto h_bc = mesh.next_halfedge(h_ab);
	const auto vert_c = mesh.to_vertex(h_bc);

	// vertex coordinates
	const auto& Xa = mesh.position(vert_a);
	const auto& Xb = mesh.position(vert_b);
	const auto& Xc = mesh.position(vert_c);

	// edges ab and bc
	const double AB[] = {Xb[0]-Xa[0], Xb[1]-Xa[1]};
	const double BC[] = {Xc[0]-Xb[0], Xc[1]-Xb[1]};

	// measure the angle at vertex b
	const double dot = AB[0]*BC[0] + AB[1]*BC[1];
	const double det = AB[0]*BC[1] - AB[1]*BC[0];
	angles[count]    = std::atan2(det, dot);
	if(angles[count]<0.) angles[count] += 2.*M_PI;

	// next
	h_ab = h_bc;
      }
    
    return (180./M_PI)*(*std::min_element(angles.begin(), angles.end()));
  }
  
  // measure quality of a vertex as the face as the ratio of smallest and largest edge lengths
  double vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert)
  {
    auto halfedge_circulator = mesh.halfedges(vert);
    std::vector<double> edge_lengths{};
    
    for(auto h:halfedge_circulator)
      {
	// vertices
	const auto vert_a = mesh.from_vertex(h);
	const auto vert_b = mesh.to_vertex(h);
	
	// coordinates
	const auto Xa = mesh.position(vert_a);
	const auto Xb = mesh.position(vert_b);

	// length
	edge_lengths.push_back( std::sqrt((Xa[0]-Xb[0])*(Xa[0]-Xb[0]) + (Xa[1]-Xb[1])*(Xa[1]-Xb[1])) );
      }
    double max_len = *std::max_element(edge_lengths.begin(), edge_lengths.end());
    double min_len = *std::min_element(edge_lengths.begin(), edge_lengths.end());
    
    return min_len/max_len;
  }
}
