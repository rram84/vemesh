// Sriramajayam

#include <vm_quality_evaluator.h>

namespace vm
{

  // Evaluate the quality of a face
  double QualityEvaluator::operator()(const std::vector<pmp::Point>& pts) const
  { return fqFunc(pts); }
  
  // Evaluate the quality of a face in a mesh
  double QualityEvaluator::operator()(const pmp::Face& face, const pmp::SurfaceMesh& mesh) const
  {
    // vertex coordinates
    std::vector<pmp::Point> coords{};
    auto v_circulator = mesh.vertices(face);
    for(auto v:v_circulator)
      coords.push_back(mesh.position(v));

    return fqFunc(coords);
  }
  
  // Evaluate the quality of a vertex in a mesh
  double QualityEvaluator::operator()(const pmp::Vertex& vertex, const pmp::SurfaceMesh& mesh) const
  {
    // loop over incident faces and return the smallest quality among them
    double quality = std::numeric_limits<double>::max();
    auto f_circulator = mesh.faces(vertex);
    for(auto f:f_circulator)
      {
  	double min_quality = (*this)(f, mesh);
	if(min_quality<quality)
	  quality = min_quality;
      }
    return quality;
  }

}
