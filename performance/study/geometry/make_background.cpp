// Sriramajayam

/** \file make_background.cpp
 * \brief Study tool: generate fixed structured background meshes (triangle and
 *        quad) over a square domain for the embedded-geometry study.
 * \ingroup performance_examples
 * \author Ramsharan Rangarajan
 */

// Generate a structured background mesh over [lo,hi]^2 with N cells across each
// direction (so (N+1)x(N+1) nodes), written as OFF for embedding with the study
// pipeline. Two element types:
//   quad : (N x N) quadrilaterals              (via create_rectangle_mesh)
//   tri  : (2 N x N) triangles                 (each cell split in two)
// The same base mesh is reused across all geometries; per-realization node
// perturbations are applied later by the embedder.
//
// Usage:
//   make_background -n 64 -t both -o out_dir            # bg_tri_64.off, bg_quad_64.off
//   make_background -n 128 -t quad -o out_dir --lo -1 --hi 1

#include <vm_tutorial_rectangle_mesh.h>   // vm::tutorial::create_rectangle_mesh
#include <vm_io.h>                          // vm::write_off

#include <CLI11.hpp>

#include <pmp/surface_mesh.h>

#include <array>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace
{
  // Structured triangle mesh over [lo,hi]^2 with N cells across (2 tris/cell).
  pmp::SurfaceMesh make_tri(double lo, double h, int N)
  {
    pmp::SurfaceMesh mesh;
    const int nx = N + 1;
    std::vector<pmp::Vertex> v(static_cast<std::size_t>(nx) * nx);
    for(int j = 0; j < nx; ++j)
      for(int i = 0; i < nx; ++i)
        v[nx * j + i] = mesh.add_vertex(pmp::Point(lo + i * h, lo + j * h, 0.));

    for(int j = 0; j < N; ++j)
      for(int i = 0; i < N; ++i)
        {
          const pmp::Vertex v00 = v[nx * j + i];
          const pmp::Vertex v10 = v[nx * j + (i + 1)];
          const pmp::Vertex v11 = v[nx * (j + 1) + (i + 1)];
          const pmp::Vertex v01 = v[nx * (j + 1) + i];
          mesh.add_triangle(v00, v10, v11);   // CCW
          mesh.add_triangle(v00, v11, v01);
        }
    return mesh;
  }
} // namespace


int main(int argc, char** argv)
{
  CLI::App app{"Generate structured triangle/quad background meshes over [lo,hi]^2"};
  app.footer("Sample usage:\n"
             "  ./make_background -n 64 -t both -o out_dir");

  int N = 64;
  std::string type = "both";
  std::string outdir;
  double lo = -1.0, hi = 1.0;

  app.add_option("-n", N, "cells across each direction ((N+1)^2 nodes)")->required()->check(CLI::PositiveNumber);
  app.add_option("-t", type, "element type: tri | quad | both")
     ->check(CLI::IsMember({"tri", "quad", "both"}));
  app.add_option("-o", outdir, "output directory")->required();
  app.add_option("--lo", lo, "domain lower corner coordinate");
  app.add_option("--hi", hi, "domain upper corner coordinate");

  CLI11_PARSE(app, argc, argv);

  if(hi <= lo)
    { std::cerr << "error: --hi must exceed --lo" << std::endl; return 1; }

  fs::create_directories(outdir);
  const double h = (hi - lo) / static_cast<double>(N);

  const bool do_tri  = (type == "tri"  || type == "both");
  const bool do_quad = (type == "quad" || type == "both");

  if(do_quad)
    {
      // (N+1)x(N+1) nodes -> N x N quads; create_rectangle_mesh sets domain_id/interface_id
      pmp::SurfaceMesh q = vm::tutorial::create_rectangle_mesh({lo, lo}, h, N + 1, h, N + 1);
      const std::string f = (fs::path(outdir) / ("bg_quad_" + std::to_string(N) + ".off")).string();
      vm::write_off(q, f);
      std::cout << "quad: " << q.n_vertices() << " verts, " << q.n_faces()
                << " quads -> " << f << std::endl;
    }

  if(do_tri)
    {
      pmp::SurfaceMesh t = make_tri(lo, h, N);
      const std::string f = (fs::path(outdir) / ("bg_tri_" + std::to_string(N) + ".off")).string();
      vm::write_off(t, f);
      std::cout << "tri:  " << t.n_vertices() << " verts, " << t.n_faces()
                << " tris  -> " << f << std::endl;
    }

  std::cout << "domain [" << lo << "," << hi << "]^2, N=" << N << ", h=" << h << std::endl;
  return 0;
}
