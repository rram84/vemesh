// Sriramajayam

/** \file clip_circle.cpp
 * \brief Performance-test generator: clips a structured quad mesh to a circular
 *        disk at a specified refinement level.
 *        Applies random mesh perturbations near the interface to realize various intersection scenarios.
 *        Writes each result as VTK for improvement with vemesh_app.
 * \ingroup performance_examples
 * \author Ramsharan Rangarajan
 */

// Clip a structured quad mesh to the disk bounded by a circle, generating many
// realizations by randomly perturbing the nodes near the circle.
// Each clipped mesh is saved (VTK) so it can subsequently be improved with vemesh_app.
//
// Pipeline per realization:
//   1. generate a background structured mesh with specified refinement
//   2. loop over mesh realizations
//   3. randomly perturb mesh nodes in the vicinity of the circle
//   4. push perturbed nodes off the zero level set (adjust_mesh_nodes)
//   5. clip the mesh to the disk interior (clip_mesh): keeps phi<0, slices cut cells
//   6. save the clipped mesh as <outdir>/clip-<iter>.vtk
//
// Options
//   -o  output directory (created if needed)
//   -n  number of realizations to generate (default 10)
//   -d  background-mesh refinement level, >= 0 (default 0): level 0 is h=0.2 /
//       ncount=10; each level halves h and doubles ncount
//   -S  RNG seed for a reproducible mesh set (default: random, printed at startup)

#include <vm_tutorial_rectangle_mesh.h>  // vm::tutorial::create_rectangle_mesh
#include <vm_tutorial_mesh_slicer.h>    // vm::tutorial::{LevelSetFn,adjust_mesh_nodes,clip_mesh}
#include <vm_io.h>                      // vm::write_vtk
#include <vm_mesh_inspection.h>         // vm::inspect_mesh, vm::MeshInspection

#include <CLI11.hpp>

#include <random>
#include <filesystem>
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <cmath>

namespace fs = std::filesystem;

int main(int argc, char** argv)
{
  // command-line options
  CLI::App app{"Clip a structured square mesh to a circular disk"};
  app.footer("Sample usage:\n ./clip_circle -o outdir -d 2 -n 10");

  std::string outdir;
  int num_realizations = 10;
  int subdivision = 0;
  unsigned int seed = std::random_device{}();  // default: nondeterministic

  app.add_option("-o", outdir, "output directory")->required();
  app.add_option("-n", num_realizations, "number of realizations to generate")->check(CLI::PositiveNumber);
  app.add_option("-d", subdivision, "background-mesh refinement level (>= 0)")->check(CLI::NonNegativeNumber);
  app.add_option("-S", seed, "RNG seed for a reproducible mesh set (default: random)");

  CLI11_PARSE(app, argc, argv);

  // output directory
  fs::create_directories(outdir);

  // signed-distance level set to circle
  const double radius = 0.45;
  vm::tutorial::LevelSetFn sdfunc = [&radius](const double* X) {
    return std::sqrt(X[0]*X[0]+X[1]*X[1])-radius;
  };

  // echo the run configuration (seed is printed so a random run can be reproduced with -S)
  std::cout << "clip_circle run configuration:\n"
	    << "  circle radius          : " << radius << "\n"
	    << "  output directory   (-o): " << outdir           << "\n"
	    << "  realizations       (-n): " << num_realizations << "\n"
	    << "  refinement level   (-d): " << subdivision      << "\n"
	    << "  RNG seed           (-S): " << seed             << "\n";

 
  // background grid for the requested refinement level
  const std::array<double,2> left_cnr = {-1.0,-1.0};
  const double hval   = 0.2 / std::pow(2, subdivision);
  const int    ncount = 10  * std::pow(2, subdivision);

  // pre-seeded random number generator
  std::mt19937 generator(seed);

  std::cout << "background mesh  : level " << subdivision
	    << ", h = " << hval << ", ncount = " << ncount << "\n";

  // structured background mesh
  auto sq_mesh = vm::tutorial::create_rectangle_mesh(left_cnr, hval, ncount, hval, ncount);

  // nodes near the circle
  std::vector<pmp::Vertex> proximal_vertices{};
  for(auto v : sq_mesh.vertices())
    if(!sq_mesh.is_boundary(v))
      {
	const auto& X = sq_mesh.position(v);
	const double Y[] = {X[0], X[1]};
	if(std::abs(sdfunc(Y)) < 1.25 * hval)
	  proximal_vertices.push_back(v);
      }

  // perturbation parameters
  const double phi_tol  = 1.e-5;
  const double pert_tol = 10. * phi_tol;

  std::uniform_real_distribution<double> distribution(-0.15 * hval, 0.15 * hval);

  // generate realizations
  for(int iter = 0; iter < num_realizations; ++iter)
    {
      std::cout << "Realization: " << iter << std::endl;

      // perturb mesh nodes near the circle
      pmp::SurfaceMesh pert_mesh = sq_mesh;
      for(const auto& v : proximal_vertices)
	{
	  auto& X = pert_mesh.position(v);
	  X[0] += distribution(generator);
	  X[1] += distribution(generator);
	}

      pmp::SurfaceMesh circ_mesh;
      try
	{
	  // push perturbed nodes off the zero level set
	  vm::tutorial::adjust_mesh_nodes(pert_mesh, phi_tol, pert_tol, sdfunc);

	  // reject perturbations that produced an invalid background mesh
	  if(!vm::inspect_mesh(pert_mesh, vm::MeshInspection::Adjacency))
	    {
	      std::cout << "  generated an invalid mesh, redoing perturbation" << std::endl;
	      --iter;
	      continue;
	    }

	  // clip the perturbed mesh to the disk interior (phi < 0)
	  circ_mesh = vm::tutorial::clip_mesh(pert_mesh, phi_tol, sdfunc);
	}
      catch(const std::exception& e)
	{
	  std::cout << "  realization failed (" << e.what() << "), redoing" << std::endl;
	  --iter;
	  continue;
	}

      // save the clipped mesh
      vm::write_vtk(circ_mesh,
		    (fs::path(outdir) / ("clip-" + std::to_string(iter) + ".vtk")).string());
    }

  return 0;
}
