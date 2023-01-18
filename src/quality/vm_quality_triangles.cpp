// Sriramajayam

#include <vm_quality.h>

namespace vm
{
  std::map<int, double> get_triangle_qualities_map(const pmp::SurfaceMesh& mesh)
  {
    std::map<int,double> qualities{};
    auto f_circulator = mesh.faces();
    for(auto f:f_circulator)
      if(mesh.valence(f)==3)
	qualities.insert({f.idx(), compute_angle_based_face_quality(mesh, f)});

    return qualities;
  }
  
  std::set<double> get_triangle_qualities_set(const pmp::SurfaceMesh& mesh)
  {
    std::set<double> qualities{};
    auto f_circulator = mesh.faces();
    for(auto f:f_circulator)
      if(mesh.valence(f)==3)
	qualities.insert(compute_angle_based_face_quality(mesh, f));

    return qualities;
  }
}
