// Sriramajayam

#include <vm_quality.h>

namespace vm
{  
  std::list<std::pair<int,double>> get_triangle_qualities(const pmp::SurfaceMesh& mesh)
  {
    std::list<std::pair<int,double>> qualities{};
    auto f_circulator = mesh.faces();
    for(auto f:f_circulator)
      if(mesh.valence(f)==3)
	qualities.push_back({f.idx(),compute_angle_based_face_quality(mesh, f)});

    // sort the list by element quality
    qualities.sort( [](const std::pair<int,double>& A, const std::pair<int,double>& B){return A.second<B.second;} ); 
    return qualities;
  }
}
