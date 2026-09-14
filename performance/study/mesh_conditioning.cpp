// Sriramajayam

/** \file mesh_conditioning.cpp
 * \brief Study tool: global VEM stiffness conditioning of one or more meshes,
 *        a drop-in replacement for the MATLAB vem_eig pass.
 *
 * For each mesh it reports the conditioning of the lowest-order (k=1)
 * pure-Neumann VEM Poisson stiffness (unit stabilization):
 *     ratio = lambda_max / lambda_2
 * where lambda_2 is the smallest NONZERO eigenvalue (the constant mode gives an
 * eigenvalue ~0). This mirrors performance/matlab/vem_quality.m but assembles
 * from vm::quality::vem_stiffness_matrix, the SAME element stiffness the
 * library's stability ratio uses, so the global conditioning and the element
 * metric share one definition. The assembly/eigensolve live in
 * vm_study_conditioning.{h,cpp}; correctness is covered by test_conditioning.
 *
 * Output (one CSV line per file, matching vem_eig for a drop-in swap):
 *     <basename>.vtk,<lambda_2>,<lambda_max>,<ratio>
 *
 * \author Ramsharan Rangarajan
 */

#include "vm_study_conditioning.h"

#include <vm_io.h>
#include <CLI11.hpp>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
  std::vector<std::string> files;

  CLI::App app{"Global VEM stiffness conditioning (C++ vem_eig replacement)"};
  app.add_option("files", files, "input mesh files (.vtk)")
    ->required()->check(CLI::ExistingFile);
  CLI11_PARSE(app, argc, argv);

  std::cout.setf(std::ios::scientific);
  std::cout.precision(17);

  for(const auto& fp : files)
  {
    const pmp::SurfaceMesh mesh = vm::read_vtk(fp);
    const vm::study::Conditioning c = vm::study::vem_conditioning(mesh);
    const std::string name = std::filesystem::path(fp).filename().string();
    std::cout << name << ',' << c.lambda_2 << ',' << c.lambda_max << ',' << c.ratio << '\n';
  }
  return 0;
}
