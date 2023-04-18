// Sriramajayam

#include <vm_vertex_quality.h>
#include <vm_face_quality.h>
#include <limits>

namespace vm
{
  // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
  // smallest nonzero eigenvalue
  double compute_stiffness_based_vertex_quality(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert)
  {
    // loop over incident faces
    // return the smallest eigenvalue encountered
    double quality = std::numeric_limits<double>::max();
    auto f_circulator = mesh.faces(vert);
    for(auto f:f_circulator)
      {
  	double min_eigval = compute_stiffness_based_mesh_face_quality(mesh, f);
	if(min_eigval<quality)
	  quality = min_eigval;
      }
    return quality;
  }
  
}
