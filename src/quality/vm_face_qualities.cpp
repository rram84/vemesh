// Sriramajayam

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


    namespace {
      // compute the angle included by the pair of segments joining three points
      /*
       * u    w
       * \  /
       *  v
       */
      double compute_included_angle_in_degrees(const pmp::Point& U, const pmp::Point& V, const pmp::Point& W)
      {
	// edges
	const double VU[] = {U[0]-V[0], U[1]-V[1]};
	const double VW[] = {W[0]-V[0], W[1]-V[1]};

	// measure the angle at vertex V
	const double dot = VU[0]*VW[0] + VU[1]*VW[1];
	const double det = VU[0]*VW[1] - VU[1]*VW[0];
	double angle     = std::atan2(-det, dot);
	if(angle<0.)
	  angle += 2.*M_PI;
      
	return (180./M_PI)*angle;
      }
    }

    
    // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
    // smallest included angle
    double geom_min_angle(const std::vector<pmp::Point>& coords)
    {
      const int nverts = static_cast<int>(coords.size());
      double min_angle = 360.;
      for(int a=0; a<nverts; ++a)
	{
	  const auto& Xa = coords[a];
	  const auto& Xb = coords[(a+1)%nverts];
	  const auto& Xc = coords[(a+2)%nverts];
	  
	  // angle between edges ab and bc
	  const double angle = compute_included_angle_in_degrees(Xa, Xb, Xc);
	  
	  // track the minimum
	  if(angle<min_angle)
	    min_angle = angle;
	}
      return min_angle;
    }
    
  }
}
