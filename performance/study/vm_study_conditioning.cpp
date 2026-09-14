// Sriramajayam

/** \file vm_study_conditioning.cpp
 * \brief Implementation of global VEM stiffness assembly + conditioning.
 * \author Ramsharan Rangarajan
 */

#include "vm_study_conditioning.h"

#include <Spectra/SymEigsSolver.h>
#include <Spectra/SymEigsShiftSolver.h>
#include <Spectra/MatOp/SparseSymMatProd.h>
#include <Spectra/MatOp/SparseSymShiftSolve.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace vm
{
  namespace study
  {
    Eigen::SparseMatrix<double> assemble_vem_stiffness(const pmp::SurfaceMesh& mesh)
    {
      const int n = static_cast<int>(mesh.n_vertices());
      std::vector<Eigen::Triplet<double>> trips;
      trips.reserve(static_cast<std::size_t>(n) * 8);

      std::vector<pmp::Point> coords;
      std::vector<int> vidx;
      for(auto f : mesh.faces())
      {
        coords.clear(); vidx.clear();
        for(auto v : mesh.vertices(f)) { coords.push_back(mesh.position(v)); vidx.push_back(v.idx()); }

        const Eigen::MatrixXd Ke = vm::quality::vem_stiffness_matrix(coords, 1.0);
        const int m = static_cast<int>(coords.size());
        for(int a=0; a<m; ++a)
          for(int b=0; b<m; ++b)
            trips.emplace_back(vidx[a], vidx[b], Ke(a,b));
      }

      Eigen::SparseMatrix<double> K(n, n);
      K.setFromTriplets(trips.begin(), trips.end());   // sums duplicate entries
      K.makeCompressed();
      return K;
    }

    Eigen::SparseMatrix<double> symmetrized(const Eigen::SparseMatrix<double>& K)
    {
      Eigen::SparseMatrix<double> Kt = Eigen::SparseMatrix<double>(K.transpose());
      Eigen::SparseMatrix<double> S = 0.5 * (K + Kt);
      S.makeCompressed();
      return S;
    }

    double largest_eigenvalue(const Eigen::SparseMatrix<double>& K)
    {
      Spectra::SparseSymMatProd<double> op(K);
      const int ncv = std::min<int>(K.rows(), 12);
      Spectra::SymEigsSolver<Spectra::SparseSymMatProd<double>> eigs(op, 1, ncv);
      eigs.init();
      eigs.compute(Spectra::SortRule::LargestAlge);
      if(eigs.info() != Spectra::CompInfo::Successful)
        throw std::runtime_error("largest_eigenvalue: Lanczos did not converge");
      return eigs.eigenvalues()(0);
    }

    std::array<double,2> smallest_two_eigenvalues(const Eigen::SparseMatrix<double>& K,
                                                  double lambda_max)
    {
      const double sigma = -1e-12 * lambda_max;          // slightly negative -> (K-sigma I) SPD
      Spectra::SparseSymShiftSolve<double> op(K);
      const int ncv = std::min<int>(K.rows(), 16);
      Spectra::SymEigsShiftSolver<Spectra::SparseSymShiftSolve<double>> eigs(op, 2, ncv, sigma);
      eigs.init();
      eigs.compute(Spectra::SortRule::LargestMagn);      // nearest to sigma
      if(eigs.info() != Spectra::CompInfo::Successful)
        throw std::runtime_error("smallest_two_eigenvalues: shift-invert did not converge");
      Eigen::VectorXd ev = eigs.eigenvalues();
      std::sort(ev.data(), ev.data() + ev.size());
      return {ev(0), ev(1)};
    }

    Conditioning vem_conditioning(const pmp::SurfaceMesh& mesh)
    {
      const Eigen::SparseMatrix<double> K = symmetrized(assemble_vem_stiffness(mesh));
      const double lambda_max = largest_eigenvalue(K);
      const std::array<double,2> lo = smallest_two_eigenvalues(K, lambda_max);
      const double lambda_2 = lo[1];
      return { lambda_2, lambda_max, lambda_max / lambda_2 };
    }
  }
}
