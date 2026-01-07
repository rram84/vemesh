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
      
      // first value should be approximately zero, subsequent should be positive
      assert(eigarray[1]>eigarray[0]);
      
      // return the second eigenvalue
      return eigarray[1]/eigarray[nvals-1];
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
