// Sriramajayam

/** \file embed_check.cpp
 * \brief Study tool: test whether a geometry embeds ROBUSTLY into a fixed
 *        background mesh under small node perturbations (admissibility gate).
 * \ingroup performance_examples
 * \author Ramsharan Rangarajan
 */

// Decide whether a closed geometry is admissible for the embedded-geometry study
// on a given fixed background mesh. Because the production runs perturb the
// background nodes near the interface, admissibility must be robust to those
// perturbations: a geometry whose thin neck lets two branches fall in one cell
// under a small jiggle must be rejected, not merely a geometry that fails an
// unperturbed embed.
//
// The test therefore runs K independent perturbed trials with the SAME scheme as
// embed_shapes (proximal nodes within `proximity`*h moved by uniform(-alpha*h,
// alpha*h), then adjust_mesh_nodes pushes nodes off the zero level set, then
// embed_interface). The geometry PASSES only if EVERY trial yields a valid
// background and a valid embedded mesh; it FAILS (is rejected) on the first
// invalid trial.
//
// Output: a single line to stdout,
//   PASS <geom> <bg> trials=<K>
//   FAIL <geom> <bg> trial=<k> reason=<invalid_background|invalid_embedded|exception:...>
// and an exit code (0 = PASS, 1 = FAIL) so a driver can tally with $?.
//
// Usage:
//   embed_check -g shape.dat -i bg_quad_64.off [-n 25] [-S 12345]
//               [--alpha 0.15] [--proximity 1.25]

#include <vm_tutorial_mesh_slicer.h>   // adjust_mesh_nodes, embed_interface
#include <vm_tutorial_polygonSDF.h>    // PolygonSDF, read_polygon_loops
#include <vm_io.h>                      // read_off
#include <vm_mesh_inspection.h>         // inspect_mesh, MeshInspection

#include <CLI11.hpp>

#include <cmath>
#include <exception>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace
{
  // average edge length of a mesh (matches embed_shapes)
  double mesh_size(const pmp::SurfaceMesh& mesh)
  {
    double total = 0.; int count = 0;
    for(auto e : mesh.edges())
      {
        const auto& a = mesh.position(mesh.vertex(e, 0));
        const auto& b = mesh.position(mesh.vertex(e, 1));
        const double dx = a[0] - b[0], dy = a[1] - b[1];
        total += std::sqrt(dx * dx + dy * dy);
        ++count;
      }
    return count > 0 ? total / count : 0.;
  }
} // namespace


int main(int argc, char** argv)
{
  CLI::App app{"Robust admissibility check: does a geometry embed under perturbation?"};
  app.footer("Sample usage:\n"
             "  ./embed_check -g shape.dat -i bg_quad_64.off -n 25");

  std::string geom_file, bg_file;
  int    trials     = 25;
  unsigned int seed = 12345;           // fixed default -> reproducible sweep
  double alpha      = 0.15;            // perturbation magnitude as fraction of h
  double proximity  = 1.25;           // proximal-node band as multiple of h

  app.add_option("-g", geom_file, "geometry .dat (closed loops)")->required()->check(CLI::ExistingFile);
  app.add_option("-i", bg_file,   "background mesh .off")->required()->check(CLI::ExistingFile);
  app.add_option("-n", trials,    "number of perturbed trials")->check(CLI::PositiveNumber);
  app.add_option("-S,--seed", seed, "RNG seed");
  app.add_option("--alpha", alpha, "perturbation magnitude as fraction of edge length");
  app.add_option("--proximity", proximity, "proximal-node band as multiple of edge length");

  CLI11_PARSE(app, argc, argv);

  const std::string geom = geom_file;   // labels for the result line
  const std::string bg   = bg_file;

  // interface signed-distance from the (possibly multi-loop) geometry
  vm::tutorial::PolygonSDF interface_sdf(vm::tutorial::read_polygon_loops(geom_file));
  vm::tutorial::LevelSetFn sdfunc = [&interface_sdf](const double* X) { return interface_sdf(X); };

  // fixed base background
  const pmp::SurfaceMesh base = vm::read_off(bg_file);
  const double hval = mesh_size(base);

  // proximal (non-boundary) nodes near the interface, on the UNPERTURBED base
  std::vector<pmp::Vertex> proximal;
  for(auto v : base.vertices())
    if(!base.is_boundary(v))
      {
        const auto& X = base.position(v);
        const double Y[] = {X[0], X[1]};
        if(std::abs(sdfunc(Y)) < proximity * hval)
          proximal.push_back(v);
      }

  const double phi_tol  = 1.e-5;
  const double pert_tol = 10. * phi_tol;

  std::mt19937 gen(seed);
  std::uniform_real_distribution<double> dist(-alpha * hval, alpha * hval);

  auto fail = [&](int k, const std::string& reason) {
    std::cout << "FAIL " << geom << " " << bg << " trial=" << k
              << " reason=" << reason << std::endl;
    return 1;
  };

  for(int k = 0; k < trials; ++k)
    {
      pmp::SurfaceMesh mesh = base;                 // handles from base valid on the copy
      for(const auto& v : proximal)
        {
          auto& X = mesh.position(v);
          X[0] += dist(gen);
          X[1] += dist(gen);
        }
      try
        {
          vm::tutorial::adjust_mesh_nodes(mesh, phi_tol, pert_tol, sdfunc);
          if(!vm::inspect_mesh(mesh, vm::MeshInspection::Adjacency))
            return fail(k, "invalid_background");
          pmp::SurfaceMesh embedded = vm::tutorial::embed_interface(mesh, phi_tol, sdfunc);
          if(!vm::inspect_mesh(embedded, vm::MeshInspection::Adjacency))
            return fail(k, "invalid_embedded");
        }
      catch(const std::exception& e)
        {
          return fail(k, std::string("exception:") + e.what());
        }
    }

  std::cout << "PASS " << geom << " " << bg << " trials=" << trials << std::endl;
  return 0;
}
