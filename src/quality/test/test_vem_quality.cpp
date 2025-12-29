// Sriramajayam

#include <vm_face_qualities.h>
#include <iostream>


// Compare two sorted vectors
bool eigenvalues_close(Eigen::VectorXd a,
		       Eigen::VectorXd b,
		       double tol)
{
  if (a.size() != b.size())
    return false;
  
  std::sort(a.data(), a.data() + a.size());
  std::sort(b.data(), b.data() + b.size());

  return (a - b).norm() <= tol;
}

// triangle stiffness matrix with FE
Eigen::MatrixXd fe_triangle_stiffness(const std::vector<pmp::Point>& coords);

int main()
{
  // ----------- test 1: compare with Sutton's output --------- //
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

    // VEM stiffness matrix
    auto K = vm::quality::vem_stiffness_matrix(coords, 1.0);

    // check symmetry
    if (!K.isApprox(K.transpose(), 1.e-8))
      {
	std::cerr << "\nStiffness matrix is not symmetric\n" << std::flush;
	return EXIT_FAILURE;
      }
    
    // eigenvalues
    auto evals = K.selfadjointView<Eigen::Lower>().eigenvalues();
    
    // Sutton's matrix
    Eigen::MatrixXd Kref(5,5);
    Kref <<
      1.6211,  -1.2102,  -0.6726,  -0.5041,   0.7658,
      -1.2102,   1.3357,   0.2502,   0.0965,  -0.4722,
      -0.6726,   0.2502,   0.8221,   0.2586,  -0.6583,
      -0.5041,   0.0965,   0.2586,   1.3674,  -1.2185,
      0.7658,  -0.4722,  -0.6583,  -1.2185,   1.5832;

    // Eigenvalues of Sutton's matrix
    Eigen::VectorXd evals_ref(5);
    evals_ref << 3.9703, 1.8015, 0.6682, 0.2896, 0.0; // last = null vector (rigid body mode)

    // Tolerances
    const double mat_tol = 1e-3;
    const double eig_tol = 1e-4;
    
    // Checks
    if(!K.isApprox(Kref, mat_tol))
      {
	std::cerr << "Stiffness matrix mismatch\n" << std::flush;
	return EXIT_FAILURE;
      }
    
    if(!eigenvalues_close(evals, evals_ref, eig_tol))
      {
	std::cerr << "Eigenvalue mismatch\n" << std::flush;
	return EXIT_FAILURE;
      }

    // stability ratio
    double stab_ratio = vm::quality::vem_stability_ratio(coords);
    if(std::abs(stab_ratio-0.2896/3.9703)>eig_tol)
      {
	std::cerr << "Stability ratio mismatch\n" << std::flush;
	return EXIT_FAILURE;
      }
    
  }

  // ------ test 2: VEM stiffness for a triangle with 0 stabilization = stiffness with FE ---- //
  {
    // create a triangle
    std::vector<pmp::Point> coords(3);
    coords[0] = pmp::Point({0.,0.,0.});
    coords[1] = pmp::Point({1.,0.,0.}); 
    coords[2] = pmp::Point({0.,1.,0.});

    // VEM stiffness matrix with no stabilization
    auto Kvem = vm::quality::vem_stiffness_matrix(coords, 0.0);

    // check symmetry
    if (!Kvem.isApprox(Kvem.transpose(), 1.e-8))
      {
	std::cerr << "\nStiffness matrix is not symmetric\n" << std::flush;
	return EXIT_FAILURE;
      }
    
    // verify with direct computation
    auto Kfe = fe_triangle_stiffness(coords);
    
    // tolerances
    constexpr double mat_tol  = 1e-6;
    constexpr double eig_tol  = 1e-6;

    // compare matrices
    if (!Kvem.isApprox(Kfe, mat_tol))
      {
	std::cerr << "Stiffness matrix mismatch\n" << std::flush;
	return EXIT_FAILURE;
      }
 
    // compare eigenvalues (order independent)
    Eigen::VectorXd eig_vem =
      Kvem.selfadjointView<Eigen::Lower>().eigenvalues();

    Eigen::VectorXd eig_fe =
      Kfe.selfadjointView<Eigen::Lower>().eigenvalues();

    if (!eigenvalues_close(eig_vem, eig_fe, eig_tol))
      {
	std::cerr << "Eigenvalue mismatch\n" << std::flush;
	return EXIT_FAILURE;
      }
 
  }
  
}


// triangle area
double triangle_area(const std::vector<pmp::Point>& coords)
{
  const double U[] = {coords[1][0]-coords[0][0], coords[1][1]-coords[0][1]};
  const double V[] = {coords[2][0]-coords[0][0], coords[2][1]-coords[0][1]};
  return 0.5*(U[0]*V[1]-U[1]*V[0]);
}

// triangle stiffness matrix
Eigen::MatrixXd fe_triangle_stiffness(const std::vector<pmp::Point>& coords)
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
