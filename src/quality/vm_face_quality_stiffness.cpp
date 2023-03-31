// Sriramajayam

#include <vm_face_quality.h>
#include <vm_polygon_stiffness.h>
#include <iostream>
namespace vm
{
  // measure quality of a face as the smallest nonzero eigenvalue of the vem stiffness matrix
  double compute_stiffness_based_face_quality(const pmp::SurfaceMesh& mesh, const pmp::Face& face)
  {
    // vertex coordinates
    std::vector<std::array<double,2>> coords{};
    auto v_circulator = mesh.vertices(face);
    for(auto v:v_circulator)
      {
	const auto& X = mesh.position(v);
	coords.push_back({X[0],X[1]});
      }

    // stiffness matrix
    auto Kmat = compute_polygon_stiffness_matrix(coords, 1.0);
    
    // eigenvalues
    auto eigvals = Kmat.selfadjointView<Eigen::Lower>().eigenvalues();
    const int nvals = eigvals.rows();
    auto* eigarray = eigvals.data();
    std::sort(eigarray, eigarray+nvals);

    // first value should be zero, subsequent should be positive
    assert(std::abs(eigarray[0])<1.e-6 && eigarray[1]>0.);

    // return the second eigenvalue
    return eigarray[1];
  }
  
}
