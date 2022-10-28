// Sriramajayam

#include <vm_move.h>
#include <vm_inspect.h>
#include <limits>

namespace vm
{
  // examine whether a given vertex needs to be moved
  bool needs_move(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex,
		  const double eps_length_ratio)
  {
    assert(mesh.is_valid(vertex) && !mesh.is_deleted(vertex));
    assert(eps_length_ratio<1.);
    
    // examine lengths of half-edges around the vertex
    int nedges = 0;
    double len_sum = 0.;
    double min_len = std::numeric_limits<double>::max();
    auto halfedge_circulator = mesh.halfedges(vertex);
    const auto& Xa = mesh.position(vertex);
    for(auto h:halfedge_circulator)
      {
	assert(mesh.from_vertex(h)==vertex);
	const auto& Xb = mesh.position(mesh.to_vertex(h));
	double len = std::sqrt((Xa[0]-Xb[0])*(Xa[0]-Xb[0]) + (Xa[1]-Xb[1])*(Xa[1]-Xb[1]));
	if(len<min_len)
	  min_len = len;
	len_sum += len;
	++nedges;
      }
    double len_avg = len_sum/static_cast<double>(nedges);

    // examine the smallest edge length ratio
    double ratio   = min_len/len_avg;
    if(ratio<eps_length_ratio)
      return true;
    else
      return false;
  }

}
