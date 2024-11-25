// Sriramajayam

#include <vm_quality.h>
#include <iostream>

// triangle stiffness matrix
Eigen::MatrixXd triangle_stiffness(const std::vector<pmp::Point>& coords);

int main()
{
  // create a triangle
  std::vector<pmp::Point> coords(3);
  coords[0] = pmp::Point({0.,0.,0.});
  coords[1] = pmp::Point({1.,0.,0.}); 
  coords[2] = pmp::Point({0.,1.,0.}); 
  auto Kmat = vm::compute_polygon_stiffness_matrix(coords, 0.0);

  // verify with direct computation
  auto Ktri = triangle_stiffness(coords);
  
  std::cout << "Polygon stiffness matrix: "
	    << std::endl << Kmat << std::endl;
  std::cout << "Matrix eigenvalues: " << Kmat.selfadjointView<Eigen::Lower>().eigenvalues() << std::endl;

  std::cout<< "Triangle stiffness matrix: "
	   << std::endl << Ktri << std::endl;
  
}

// triangle area
double triangle_area(const std::vector<pmp::Point>& coords)
{
  const double U[] = {coords[1][0]-coords[0][0], coords[1][1]-coords[0][1]};
  const double V[] = {coords[2][0]-coords[0][0], coords[2][1]-coords[0][1]};
  return 0.5*(U[0]*V[1]-U[1]*V[0]);
}

// triangle stiffness matrix
Eigen::MatrixXd triangle_stiffness(const std::vector<pmp::Point>& coords)
{
  const double area = triangle_area(coords);
  
  // shape function gradient coefficients
  double b[3], c[3];
  for(int i=0; i<3; ++i)
    {
      const int j = (i+1)%3;
      const int k = (i+2)%3;
      b[i] = (coords[j][1]-coords[k][1])/(2.*area);
      c[i] = (coords[k][0]-coords[j][0])/(2.*area);
    }

  // stiffness matrix
  Eigen::MatrixXd K(3,3);
  for(int i=0; i<3; ++i)
    for(int j=0; j<3; ++j)
      K(i,j) = (b[i]*b[j]+c[i]*c[j])*area;

  return K;
}
