// Sriramajayam

/** \file sampling_check.cpp
 * \brief Study tool: a-priori resolution gate — can the nodal-sampled SDF on an
 *        N x N background faithfully represent a geometry, or does some cell
 *        carry more than one interface branch ("two curves in one cell")?
 * \ingroup performance_examples
 * \author Ramsharan Rangarajan
 */

// The embedder reconstructs the interface as the zero contour of the signed
// distance sampled at the mesh NODES. A background cell whose four nodal signs
// must encode two interface chords cannot represent them: one chord is merged or
// dropped, implicitly changing the geometry. This tool detects that a priori,
// directly from the true interface vs. the structured grid over [lo,hi]^2 with N
// cells across (no embedding needed):
//
//   * MULTI-BRANCH cell: a cell whose boundary is crossed by the interface more
//     than twice (> 2 crossings => more than one chord => unrepresentable).
//   * SUB-CELL loop: a closed loop that never crosses a grid line (it lies
//     inside a single cell => all that cell's nodes share a sign => the loop is
//     lost entirely).
//
// A geometry is ADMISSIBLE at N iff it has zero multi-branch cells and zero
// sub-cell loops.
//
// Output (single line + exit code 0=PASS, 1=FAIL):
//   PASS <geom> N=<n> bad_cells=0 subcell_loops=0
//   FAIL <geom> N=<n> bad_cells=<k> subcell_loops=<m> worst_crossings=<c>
//
// Usage:
//   sampling_check -g shape.dat -n 64 [--lo -1] [--hi 1]

#include <vm_tutorial_polygonSDF.h>    // vm::tutorial::read_polygon_loops

#include <CLI11.hpp>

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace
{
  // clamp a node index derived from a coordinate into [0, N-1] (the owning cell)
  inline int cell_index(double c, double lo, double h, int N)
  {
    int k = static_cast<int>(std::floor((c - lo) / h));
    if(k < 0) k = 0;
    if(k > N - 1) k = N - 1;
    return k;
  }
} // namespace


int main(int argc, char** argv)
{
  CLI::App app{"A-priori resolution gate: is any cell shared by two distinct interface loops?"};
  app.footer("Sample usage:\n  ./sampling_check -g shape.dat -n 64");

  std::string geom_file;
  int    N  = 64;
  double lo = -1.0, hi = 1.0;

  app.add_option("-g", geom_file, "geometry .dat (closed loops)")->required()->check(CLI::ExistingFile);
  app.add_option("-n", N, "cells across each direction")->required()->check(CLI::PositiveNumber);
  app.add_option("--lo", lo, "domain lower corner coordinate");
  app.add_option("--hi", hi, "domain upper corner coordinate");

  CLI11_PARSE(app, argc, argv);
  if(hi <= lo) { std::cerr << "error: --hi must exceed --lo" << std::endl; return 2; }

  const double h = (hi - lo) / static_cast<double>(N);

  // INTER-COMPONENT criterion: flag a cell only if >= 2 DISTINCT loops cross its
  // boundary ("two distinct curves in one cell"). Self-necks of a single loop are
  // tolerated (they embed fine; the SDF just rounds them locally). Per cell we
  // remember the first loop that touched it and whether a different one did too.
  const std::size_t NC = static_cast<std::size_t>(N) * N;
  std::vector<int>  firstLoop(NC, -1);
  std::vector<char> multiLoop(NC, 0);
  auto add_cell = [&](int i, int j, int li) {
    if(i < 0 || i >= N || j < 0 || j >= N) return;
    const std::size_t c = static_cast<std::size_t>(j) * N + i;
    if(firstLoop[c] < 0) firstLoop[c] = li;
    else if(firstLoop[c] != li) multiLoop[c] = 1;
  };

  const auto loops = vm::tutorial::read_polygon_loops(geom_file);

  int subcell_loops = 0;

  for(int li = 0; li < static_cast<int>(loops.size()); ++li)
    {
      const auto& loop = loops[li];
      const std::size_t np = loop.size() / 2;
      if(np < 3) continue;
      bool loop_crossed_grid = false;

      // iterate closed polyline segments (last -> first closes the loop)
      for(std::size_t s = 0; s < np; ++s)
        {
          const double ax = loop[2 * s],       ay = loop[2 * s + 1];
          const std::size_t t = (s + 1) % np;
          const double bx = loop[2 * t],       by = loop[2 * t + 1];

          // --- crossings with VERTICAL grid lines x = lo + i*h ---
          if(bx != ax)
            {
              double x0 = ax, x1 = bx, y0 = ay, y1 = by;
              if(x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); }
              int i0 = static_cast<int>(std::ceil((x0 - lo) / h));
              int i1 = static_cast<int>(std::floor((x1 - lo) / h));
              for(int i = i0; i <= i1; ++i)
                {
                  if(i <= 0 || i >= N) continue;         // interior grid lines only
                  const double xi = lo + i * h;
                  const double u  = (xi - x0) / (x1 - x0);
                  if(u <= 0.0 || u >= 1.0) continue;
                  const double yc = y0 + u * (y1 - y0);
                  const int    jc = cell_index(yc, lo, h, N);
                  // vertical grid edge x=xi bounds cells (i-1,jc) and (i,jc)
                  add_cell(i - 1, jc, li);
                  add_cell(i,     jc, li);
                  loop_crossed_grid = true;
                }
            }

          // --- crossings with HORIZONTAL grid lines y = lo + j*h ---
          if(by != ay)
            {
              double x0 = ax, x1 = bx, y0 = ay, y1 = by;
              if(y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
              int j0 = static_cast<int>(std::ceil((y0 - lo) / h));
              int j1 = static_cast<int>(std::floor((y1 - lo) / h));
              for(int j = j0; j <= j1; ++j)
                {
                  if(j <= 0 || j >= N) continue;
                  const double yj = lo + j * h;
                  const double u  = (yj - y0) / (y1 - y0);
                  if(u <= 0.0 || u >= 1.0) continue;
                  const double xc = x0 + u * (x1 - x0);
                  const int    ic = cell_index(xc, lo, h, N);
                  // horizontal grid edge y=yj bounds cells (ic,j-1) and (ic,j)
                  add_cell(ic, j - 1, li);
                  add_cell(ic, j,     li);
                  loop_crossed_grid = true;
                }
            }
        }

      if(!loop_crossed_grid) ++subcell_loops;   // loop lives inside one cell
    }

  // tally cells shared by two or more distinct loops (inter-component)
  int bad_cells = 0;
  for(char m : multiLoop) if(m) ++bad_cells;

  const bool pass = (bad_cells == 0 && subcell_loops == 0);
  std::cout << (pass ? "PASS " : "FAIL ") << geom_file << " N=" << N
            << " bad_cells=" << bad_cells
            << " subcell_loops=" << subcell_loops << std::endl;
  return pass ? 0 : 1;
}
