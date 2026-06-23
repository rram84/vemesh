// Sriramajayam

/** \file embed_shapes.cpp
 * \brief Performance-test generator: embeds polygonal shapes as interior
 *        interfaces into a background triangle mesh over many randomly-perturbed
 *        realizations, writing each result as VTK for improvement with vemesh_app.
 * \ingroup performance_examples
 * \author Ramsharan Rangarajan
 */

// Embed a polygonal interface into a background triangle mesh, generating many
// realizations by randomly perturbing the nodes near the interface. Each embedded
// mesh is written to disk (VTK) so it can subsequently be improved with vemesh_app.
//
// Pipeline per realization:
//   1. read polygon vertices and build a signed-distance level set to it   (once)
//   2. read a background triangle mesh                                      (once)
//   3. randomly perturb mesh nodes in the vicinity of the interface
//   4. push perturbed nodes off the zero level set (adjust_mesh_nodes)
//   5. embed the interface (embed_interface) -> tags domain_id / interface_id
//   6. save the embedded mesh as <outdir>/embed-<iter>.vtk
//
// Options
//   -g  input file of polygon vertices defining the interface (x y per line)
//   -i  input background triangle mesh (.off)
//   -o  output directory (created if needed)
//   -n  number of realizations to generate (default 10)
//   -S  RNG seed for a reproducible mesh set (default: random, printed at startup)

#include <vm_tutorial_mesh_slicer.h>   // vm::tutorial::{LevelSetFn,adjust_mesh_nodes,embed_interface}
#include <vm_tutorial_polygonSDF.h>    // vm::tutorial::PolygonSDF
#include <vm_io.h>                      // vm::read_off, vm::write_vtk
#include <vm_mesh_inspection.h>         // vm::inspect_mesh, vm::MeshInspection
#include <vm_utils.h>                   // vm::bg, vm::boost_point_t/polygon_t/linestring_t

#include <CLI/CLI.hpp>

#include <random>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

namespace fs = std::filesystem;

// average edge length of a mesh
double mesh_size(const pmp::SurfaceMesh& mesh);


int main(int argc, char** argv)
{
  // command-line options
  CLI::App app{"Embed a polygonal geometry within a triangle mesh"};
  app.footer("Sample usage:\n"
             "  ./embed_shapes -g polyvertices.dat -i bbbb-3.off -o outdir -n 10");

  std::string geom_file, in_meshfile, outdir;
  int num_realizations = 10;
  unsigned int seed = std::random_device{}();  // default: nondeterministic

  app.add_option("-g", geom_file, "input polygon vertices (interface)")->required()->check(CLI::ExistingFile);
  app.add_option("-i", in_meshfile, "input background triangle mesh (.off)")->required()->check(CLI::ExistingFile);
  app.add_option("-o", outdir, "output directory")->required();
  app.add_option("-n", num_realizations, "number of realizations to generate")->check(CLI::PositiveNumber);
  app.add_option("-S", seed, "RNG seed for a reproducible mesh set (default: random)");

  CLI11_PARSE(app, argc, argv);

  // output directory
  fs::create_directories(outdir);

  // SDF to interface
  vm::tutorial::PolygonSDF interface_sdf(geom_file);

  // signed-distance level set to the polygon
  vm::tutorial::LevelSetFn sdfunc = [&interface_sdf](const double* X) { return interface_sdf(X); };

  // background triangle mesh
  pmp::SurfaceMesh mesh = vm::read_off(in_meshfile);
  const double hval = mesh_size(mesh);

  // nodes near the interface (computed once on the unperturbed mesh)
  std::vector<pmp::Vertex> proximal_vertices{};
  for(auto v : mesh.vertices())
    if(!mesh.is_boundary(v))
      {
        const auto& X = mesh.position(v);
        const double Y[] = {X[0], X[1]};
        if(std::abs(sdfunc(Y)) < 1.25 * hval)
          proximal_vertices.push_back(v);
      }

  // perturbation parameters
  const double phi_tol  = 1.e-5;
  const double pert_tol = 10. * phi_tol;

  std::mt19937 generator(seed);
  std::uniform_real_distribution<double> distribution(-0.15 * hval, 0.15 * hval);

  // echo the run configuration (seed is printed so a random run can be reproduced with -S)
  std::cout << "embed_shapes run configuration:\n"
            << "  interface vertices (-g): " << geom_file       << "\n"
            << "  background mesh    (-i): " << in_meshfile      << "\n"
            << "  output directory   (-o): " << outdir           << "\n"
            << "  realizations       (-n): " << num_realizations << "\n"
            << "  RNG seed           (-S): " << seed             << "\n"
            << "  mesh size (avg edge len): " << hval            << std::endl;

  // generate realizations
  for(int iter = 0; iter < num_realizations; ++iter)
    {
      std::cout << "Realization: " << iter << std::endl;

      // perturb mesh nodes near the interface (handles from `mesh` are valid on the copy)
      pmp::SurfaceMesh pert_mesh = mesh;
      for(const auto& v : proximal_vertices)
        {
          auto& X = pert_mesh.position(v);
          X[0] += distribution(generator);
          X[1] += distribution(generator);
        }

      pmp::SurfaceMesh embedded_mesh;
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

          // embed the interface in the perturbed mesh
          embedded_mesh = vm::tutorial::embed_interface(pert_mesh, phi_tol, sdfunc);
        }
      catch(const std::exception& e)
        {
          std::cout << "  realization failed (" << e.what() << "), redoing" << std::endl;
          --iter;
          continue;
        }

      // save the EMBEDDED mesh (carries domain_id / interface_id for vemesh_app)
      vm::write_vtk(embedded_mesh,
                    (fs::path(outdir) / ("embed-" + std::to_string(iter) + ".vtk")).string());
    }

  return 0;
}


// average edge length of a mesh
double mesh_size(const pmp::SurfaceMesh& mesh)
{
  double total = 0.;
  int    count = 0;
  for(auto e : mesh.edges())
    {
      const auto& a = mesh.position(mesh.vertex(e, 0));
      const auto& b = mesh.position(mesh.vertex(e, 1));
      const double dx = a[0] - b[0];
      const double dy = a[1] - b[1];
      total += std::sqrt(dx * dx + dy * dy);
      ++count;
    }
  return (count > 0) ? total / count : 0.;
}
