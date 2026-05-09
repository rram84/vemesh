// Sriramajayam

/** \file vm_face_qualities.cpp
 * \brief Implementation of VEM-based and geometric face quality metrics
 * \author Ramsharan Rangarajan
 */

#include <vm_face_qualities.h>
#include <vm_utils.h>

namespace vm
{
  namespace quality
  {
    double vem_stability_ratio(const std::vector<pmp::Point>& coords)
    {
      // stiffness matrix
      auto Kmat = vem_stiffness_matrix(coords, 1.0);
    
      // eigenvalues
      auto eigvals = Kmat.selfadjointView<Eigen::Lower>().eigenvalues();
      const int nvals = eigvals.rows();
      auto* eigarray = eigvals.data();
      std::sort(eigarray, eigarray+nvals);

      // max eigen value, should be positive
      const double lambda_max = eigarray[nvals - 1];
      if (lambda_max <= 0.)
	throw std::runtime_error("vem_stability_ratio: VEM stiffness matrix has non-positive largest eigenvalue; polygon is degenerate");

      // scaled tolerance for zero eigenvalue
      const double tol = 1.e-8 * lambda_max;

      // First eigenvalue: expect 0 for null mode, ignore it.
      if (eigarray[0] < -tol)
	throw std::runtime_error("vem_stability_ratio: VEM stiffness matrix has a negative eigenvalue beyond numerical tolerance");

      // Second eigenvalue should be >= 0
      // in case of a very small value, clamp it to zero
      if (eigarray[1] < -tol)
	throw std::runtime_error("vem_stability_ratio: second eigenvalue is negative beyond numerical tolerance");
      const double lambda_min = std::max(0., eigarray[1]);

      // return the ratio
      return lambda_min/lambda_max;
    }

    
    // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
    // ratio of the area to the perimeter^2
    double geom_shape(const std::vector<pmp::Point>& coords)
    { 
      // boost polygon of this face
      boost_polygon_t poly;
      for(auto& X:coords)
	bg::append(poly.outer(), boost_point_t(X[0],X[1]));
      
      auto first_vertex = *poly.outer().begin();
      bg::append(poly.outer(), first_vertex);

      // area
      double area = bg::area(poly);
      double perim = bg::perimeter(poly);

      // normalizing factor for this polygon
      const int n = static_cast<int>(coords.size());
      const double factor = 4.*n*std::tan(M_PI/n);
      return factor*area/(perim*perim);
    }

  }
}
