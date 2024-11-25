// Sriramajayam

#include <vm_utils.h>

namespace vm {

  double MeshFaceQuality_f(const pmp::SurfaceMesh& mesh, const pmp::Face& face, const FaceQuality_f& func)  {

    // vertex coordinates
    std::vector<pmp::Point> coords{};
    auto v_circulator = mesh.vertices(face);
    for(auto v:v_circulator)
      coords.push_back(mesh.position(v));

    return func(coords);
  }
}
