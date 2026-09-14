// Sriramajayam

/** \file vm_study_conditioning.h
 * \brief Global VEM stiffness assembly and conditioning for the study tooling.
 *
 * Assembles the lowest-order (k=1) pure-Neumann VEM Poisson stiffness (unit
 * stabilization) from the library element stiffness (vm::quality::
 * vem_stiffness_matrix -- the SAME element stiffness the stability ratio uses),
 * and computes its conditioning number lambda_max / lambda_2, where lambda_2 is
 * the smallest NONZERO eigenvalue (the constant mode gives an eigenvalue ~0).
 * This mirrors performance/matlab/vem_quality.m.
 *
 * The routines operate on a pmp::SurfaceMesh (not a file) so they are unit
 * testable on in-memory meshes; the mesh_conditioning CLI is a thin wrapper.
 *
 * \author Ramsharan Rangarajan
 */

#pragma once

#include <vm_face_qualities.h>   // vm::quality::vem_stiffness_matrix
#include <pmp/surface_mesh.h>
#include <Eigen/Sparse>

#include <array>

namespace vm
{
  namespace study
  {
    /// Assemble the RAW (not symmetrized) global k=1 VEM stiffness with unit
    /// stabilization, scattering each face's element stiffness by global vertex
    /// index. The mesh must have a dense 0..n-1 vertex indexing (as produced by
    /// vm::read_vtk on a fresh mesh).
    Eigen::SparseMatrix<double> assemble_vem_stiffness(const pmp::SurfaceMesh& mesh);

    /// Symmetric copy 0.5*(K + K^T) -- kills round-off asymmetry before the
    /// eigensolve (the element stiffness is symmetric by construction).
    Eigen::SparseMatrix<double> symmetrized(const Eigen::SparseMatrix<double>& K);

    /// Largest algebraic eigenvalue (Lanczos).
    double largest_eigenvalue(const Eigen::SparseMatrix<double>& K);

    /// The two smallest eigenvalues {lambda_min, lambda_2} (ascending) via
    /// shift-invert near sigma = -eps*lambda_max: lambda_min is the ~0 constant
    /// mode, lambda_2 the smallest nonzero.
    std::array<double,2> smallest_two_eigenvalues(const Eigen::SparseMatrix<double>& K,
                                                  double lambda_max);

    /// Conditioning summary of a mesh.
    struct Conditioning { double lambda_2; double lambda_max; double ratio; };

    /// Assemble + solve: lambda_2, lambda_max, ratio = lambda_max/lambda_2.
    Conditioning vem_conditioning(const pmp::SurfaceMesh& mesh);
  }
}
