// Sriramajayam

#include <vm_face_qualities.h>
#include <vm_utils.h>

namespace vm
{
  namespace quality
  {
    namespace {
      // distance between farthest vertices
      double compute_polygon_dia(const std::vector<pmp::Point>& coords)
      {
	const int nverts = static_cast<int>(coords.size());
	double dia = 0.;
	double dist2;
	for(int i=0; i<nverts; ++i)
	  {
	    const auto& X = coords[i];
	    for(int j=i+1; j<nverts; ++j)
	      {
		const auto& Y = coords[j];
		dist2 = (X[0]-Y[0])*(X[0]-Y[0]) + (X[1]-Y[1])*(X[1]-Y[1]);
		if(dist2>dia)
		  dia = dist2;
	      }
	  }
	return std::sqrt(dia);
      }


      // centroid of the polygon
      std::array<double,2> compute_polygon_centroid(const std::vector<pmp::Point>& coords, const double area)
      {
	std::array<double,2> C{0.,0.};
	const int nverts = static_cast<int>(coords.size());
	for(int i=0; i<nverts; ++i)
	  {
	    const double& xi = coords[i][0];
	    const double& yi = coords[i][1];
	
	    const int j = (i+1)%nverts;
	    const double& xj = coords[j][0];
	    const double& yj = coords[j][1];
	
	    C[0] += (xi+xj)*(xi*yj-xj*yi);
	    C[1] += (yi+yj)*(xi*yj-xj*yi);
	  }
	C[0] /= (6.*area);
	C[1] /= (6.*area);
	return C;
      }

  
      // area of the polygon
      double compute_polygon_area(const std::vector<pmp::Point>& coords)
      {
	// create boost polygon
	boost_polygon_t poly;
	for(auto& X:coords)
	  bg::append(poly.outer(), boost_point_t(X[0],X[1]));
	bg::append(poly.outer(), boost_point_t(coords[0][0], coords[0][1]));

	return bg::area(poly);
      }

  
      // vertex normals
      std::vector<std::array<double,2>> compute_vertex_normals(const std::vector<pmp::Point>& coords)
      {
	// average un-normalized edge vectors
	const int nverts = static_cast<int>(coords.size());
	std::vector<std::array<double,2>> bvecs(nverts);
	for(int i=1; i<=nverts; ++i)
	  {
	    // left/right vertices
	    const auto& XL = coords[i-1];
	    const auto& XR = coords[(i+1)%nverts];

	    // average = vertex normal
	    bvecs[i%nverts][0] = 0.5*(XR[1]-XL[1]);
	    bvecs[i%nverts][1] = 0.5*(XL[0]-XR[0]);
	  }

	return bvecs;
      }
    }
    
    Eigen::MatrixXd vem_stiffness_matrix(const std::vector<pmp::Point>& coords, const double tau)
    {
      // # vertices
      const int nverts = static_cast<int>(coords.size());
    
      // farthest two points
      const double hE = compute_polygon_dia(coords);
        
      // area
      const double area = compute_polygon_area(coords);

      // centroid
      const auto XE = compute_polygon_centroid(coords, area);
      
      // vertex normals
      const auto bvecs = compute_vertex_normals(coords);
 
      // B matrix
      Eigen::MatrixXd B(3, nverts);
      for(int j=0; j<nverts; ++j)
	{
	  B(0,j) = 1./static_cast<double>(nverts);
	  B(1,j) = bvecs[j][0]/hE;
	  B(2,j) = bvecs[j][1]/hE;
	}

      // D matrix
      Eigen::MatrixXd D(nverts,3);
      for(int i=0; i<nverts; ++i)
	{
	  D(i,0) = 1.;
	  D(i,1) = (coords[i][0]-XE[0])/hE;
	  D(i,2) = (coords[i][1]-XE[1])/hE;
	}

      // G matrix
      Eigen::MatrixXd G = B*D;
    
      // Projector: PI
      Eigen::MatrixXd PI = G.inverse()*B;

      // 1st row of G = 0
      G.row(0).setZero();
    
      // stabilization term
      Eigen::MatrixXd stab_mat = Eigen::MatrixXd::Identity(nverts, nverts)-D*PI;

      // stiffness matrix
      return PI.transpose()*G*PI + tau*stab_mat.transpose()*stab_mat;
    }

  }
}
