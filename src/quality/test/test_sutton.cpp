// Sriramajayam

#include <vm_quality.h>
#include <iostream>

int main()
{
  // create a pentagon using sutton's coordinates (elm #3 from meshes/voronoi.mat)
  const double verts[][2] = {{0.506080911785912,   0.501898979475360},
			     {0.510859953803347,   0.500729227376839},
			     {0.535019889166854,   0.543633250093484},
			     {0.491687102932671,   0.567330813638457},
			     {0.487797441697187,   0.564717009747145}};
			    
  std::vector<pmp::Point> coords{};
  for(int p=0; p<5; ++p)
    coords.push_back(pmp::Point(verts[p][0], verts[p][1], 0.));
  
  auto Kmat = vm::compute_polygon_stiffness_matrix(coords, 1.0);
  std::cout << "Stiffness matrix: " << std::endl << Kmat << std::endl
	    << " should be close to: " << std::endl << "\n \
    1.6211   -1.2102   -0.6726   -0.5041    0.7658 \n \
    -1.2102    1.3357    0.2502    0.0965   -0.4722 \n \
    -0.6726    0.2502    0.8221    0.2586   -0.6583 \n \
    -0.5041    0.0965    0.2586    1.3674   -1.2185 \n \
     0.7658   -0.4722   -0.6583   -1.2185    1.5832 " << std::endl;
  
  std::cout << std::endl << std::endl 
	    << "Matrix eigenvalues: " << Kmat.selfadjointView<Eigen::Lower>().eigenvalues() << std::endl
	    << " should be close to  3.9703, 1.8015, 0.6682, 0.2896, -0.0000" << std::endl;
}
