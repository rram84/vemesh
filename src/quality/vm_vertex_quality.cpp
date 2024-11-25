// Sriramajayam

#include <vm_quality.h>
#include <vm_utils.h>
#include <cmath>
#include <limits>

namespace vm {
  
  // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
  // ratio of the area to the perimeter^2
  double VertexQuality::shape(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert) {

    // loop over incident faces
    // return the smallest quality among them
    double quality = std::numeric_limits<double>::max();
    auto f_circulator = mesh.faces(vert);
    for(auto f:f_circulator)
      {
  	double min_quality = FaceQuality::shape(mesh, f);
	if(min_quality<quality)
	  quality = min_quality;
      }
    return quality;
  }


  // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
  // minimum included angle
  double VertexQuality::angle(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert) {

    // loop over incident faces
    // return the smallest quality among them
    double quality = std::numeric_limits<double>::max();
    auto f_circulator = mesh.faces(vert);
    for(auto f:f_circulator)
      {
  	double min_quality = FaceQuality::angle(mesh, f);
	if(min_quality<quality)
	  quality = min_quality;
      }
    return quality;
  }


   // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
  // smallest nonzero eigenvalue
  double VertexQuality::stiffness(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert)
  {
    // loop over incident faces
    // return the smallest eigenvalue encountered
    double quality = std::numeric_limits<double>::max();
    auto f_circulator = mesh.faces(vert);
    for(auto f:f_circulator)
      {
  	double min_eigval = FaceQuality::face_stiffness(mesh, f);
	if(min_eigval<quality)
	  quality = min_eigval;
      }
    return quality;
  }
  
}
