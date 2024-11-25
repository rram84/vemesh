// Sriramajayam

#include <vm_quality.h>
#include <cmath>

namespace vm {

  double FaceQuality::stiffness(const std::vector<pmp::Point>& coords)
  {
    // stiffness matrix
    auto Kmat = compute_polygon_stiffness_matrix(coords, 1.0);
    
    // eigenvalues
    auto eigvals = Kmat.selfadjointView<Eigen::Lower>().eigenvalues();
    const int nvals = eigvals.rows();
    auto* eigarray = eigvals.data();
    std::sort(eigarray, eigarray+nvals);

    // first value should be approximately zero, subsequent should be positive
    assert(eigarray[1]>eigarray[0]);

    // return the second eigenvalue
    return eigarray[1]/eigarray[nvals-1];
  }

}
