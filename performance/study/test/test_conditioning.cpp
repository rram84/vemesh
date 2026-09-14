// Sriramajayam

/** \file test_conditioning.cpp
 * \brief Unit tests for the study VEM-stiffness assembly + conditioning
 *        (vm::study, in vm_study_conditioning.{h,cpp}).
 * \ingroup tests
 *
 * These encode -- as self-contained checks, needing no MATLAB -- the validation
 * used to cross-check the C++ conditioner against the MATLAB vem_eig reference:
 *   1. assembled stiffness is symmetric;
 *   2. it annihilates constants (K*1 = 0) -- the defining pure-Neumann property,
 *      which validates both the element values and the global scatter;
 *   3. VEM consistency / patch test: for a linear field u = a + b*x + c*y the
 *      energy u^T K u equals the exact Dirichlet energy (b^2+c^2)*area (the
 *      stabilization vanishes on linear fields);
 *   4. the spectrum from Spectra (largest_eigenvalue / smallest_two_eigenvalues)
 *      matches a dense SelfAdjointEigenSolver reference, and the smallest
 *      eigenvalue is ~0 (PSD with a single constant null mode).
 *
 * \author Ramsharan Rangarajan
 */

#include "vm_study_conditioning.h"

#include <pmp/surface_mesh.h>
#include <Eigen/Dense>

#include <cmath>
#include <iostream>

namespace {

// Structured quad grid of nx*ny cells over [0,Lx] x [0,Ly], CCW faces.
pmp::SurfaceMesh make_quad_grid(int nx, int ny, double Lx, double Ly)
{
  pmp::SurfaceMesh mesh;
  const double hx = Lx / nx, hy = Ly / ny;
  std::vector<pmp::Vertex> v((nx+1)*(ny+1));
  auto id = [nx](int i, int j){ return j*(nx+1) + i; };
  for(int j=0; j<=ny; ++j)
    for(int i=0; i<=nx; ++i)
      v[id(i,j)] = mesh.add_vertex(pmp::Point(i*hx, j*hy, 0.0));
  for(int j=0; j<ny; ++j)
    for(int i=0; i<nx; ++i)
      mesh.add_face({ v[id(i,j)], v[id(i+1,j)], v[id(i+1,j+1)], v[id(i,j+1)] });
  return mesh;
}

int failures = 0;
void check(bool ok, const std::string& what)
{
  if(!ok) { std::cerr << "FAILED: " << what << "\n" << std::flush; ++failures; }
}

} // namespace

int main()
{
  using namespace vm::study;

  // ---- assembly invariants + patch test on a quad grid ---------------------
  {
    const double Lx = 2.0, Ly = 3.0;
    const pmp::SurfaceMesh mesh = make_quad_grid(4, 5, Lx, Ly);
    const int n = static_cast<int>(mesh.n_vertices());

    const Eigen::SparseMatrix<double> Kraw = assemble_vem_stiffness(mesh);
    const Eigen::SparseMatrix<double> K = symmetrized(Kraw);

    // (1) symmetry of the raw assembled matrix
    const double sym = (Kraw - Eigen::SparseMatrix<double>(Kraw.transpose()))
                         .toDense().cwiseAbs().maxCoeff();
    check(sym < 1e-10, "assembled stiffness not symmetric (res=" + std::to_string(sym) + ")");

    // (2) constant nullspace: K*1 = 0
    const double k1 = (K * Eigen::VectorXd::Ones(n)).cwiseAbs().maxCoeff();
    check(k1 < 1e-9, "K*1 != 0 (res=" + std::to_string(k1) + ")");

    // (3) VEM consistency / patch test: linear field energy = (b^2+c^2)*area
    const double a = 0.7, b = -1.3, c = 2.1;
    Eigen::VectorXd u(n);
    for(auto vtx : mesh.vertices())
    {
      const auto& X = mesh.position(vtx);
      u(vtx.idx()) = a + b*X[0] + c*X[1];
    }
    const double energy   = u.transpose() * (K * u);
    const double expected = (b*b + c*c) * (Lx * Ly);
    check(std::abs(energy - expected) < 1e-8 * expected,
          "patch test failed: energy=" + std::to_string(energy) +
          " expected=" + std::to_string(expected));
  }

  // ---- Spectra eigenvalues vs a dense reference; PSD / single null mode -----
  {
    const pmp::SurfaceMesh mesh = make_quad_grid(6, 6, 1.0, 1.0);
    const Eigen::SparseMatrix<double> K = symmetrized(assemble_vem_stiffness(mesh));

    // dense reference spectrum (small mesh)
    Eigen::MatrixXd Kd = Eigen::MatrixXd(K);
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> es(Kd);
    Eigen::VectorXd ev = es.eigenvalues();      // ascending
    const double ref_min = ev(0);
    const double ref_2   = ev(1);
    const double ref_max = ev(ev.size()-1);

    const double lmax = largest_eigenvalue(K);
    const std::array<double,2> lo = smallest_two_eigenvalues(K, lmax);

    check(std::abs(lmax - ref_max) < 1e-8 * ref_max,
          "lambda_max mismatch vs dense");
    check(std::abs(lo[1] - ref_2) < 1e-6 * ref_2,
          "lambda_2 mismatch vs dense");
    // smallest eigenvalue is the ~0 constant mode (PSD, single null mode)
    check(std::abs(ref_min) < 1e-8 * ref_max, "smallest eigenvalue not ~0 (not PSD?)");
    check(ref_2 > 1e-6 * ref_max, "second eigenvalue not strictly positive (disconnected?)");
    check(std::abs(lo[0]) < 1e-6 * ref_max, "Spectra lambda_min not ~0");

    // conditioning convenience matches the pieces
    const Conditioning cnd = vem_conditioning(mesh);
    check(std::abs(cnd.ratio - lmax/lo[1]) < 1e-6 * (lmax/lo[1]),
          "vem_conditioning ratio inconsistent");
  }

  if(failures)
  {
    std::cerr << failures << " check(s) failed\n" << std::flush;
    return EXIT_FAILURE;
  }
  std::cout << "test_conditioning: all checks passed\n";
  return 0;
}
