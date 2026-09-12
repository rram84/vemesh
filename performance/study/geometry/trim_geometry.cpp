// Sriramajayam

/** \file trim_geometry.cpp
 * \brief Study tool: make a geometry admissible at resolution N by iteratively
 *        removing the smallest OFFENDING loop (one that shares a cell with
 *        another branch, or is lost sub-cell), protecting the main body.
 * \ingroup performance_examples
 * \author Ramsharan Rangarajan
 */

// A geometry is admissible at N iff no background cell carries more than one
// interface chord (boundary crossed > 2 times) and no loop is sub-cell (never
// crosses a grid line) — see sampling_check. When a geometry fails, we salvage
// it by removing offending loops smallest-first:
//   * offending = a loop that contributes to a multi-branch cell, OR a sub-cell loop;
//   * we never remove the largest remaining loop (the main body): if the only
//     offender is the main body (e.g. a self-neck it can't resolve at this N),
//     the geometry is UNSALVAGEABLE and dropped.
// Repeat until admissible or unsalvageable.
//
// Output: writes the trimmed geometry to -o only on success. One status line:
//   PASS <in> removed=<k> loops_left=<m>
//   FAIL <in> unsalvageable removed=<k> loops_left=<m>
// Exit 0 = admissible (possibly after trimming), 1 = unsalvageable.
//
// Usage:
//   trim_geometry -g in.dat -n 128 -o out.dat [--lo -1] [--hi 1]

#include <vm_tutorial_polygonSDF.h>    // vm::tutorial::read_polygon_loops

#include <CLI11.hpp>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace
{
  using Loop  = std::vector<double>;          // flattened x,y
  using Loops = std::vector<Loop>;

  inline int cell_index(double c, double lo, double h, int N)
  {
    int k = static_cast<int>(std::floor((c - lo) / h));
    if(k < 0) k = 0;
    if(k > N - 1) k = N - 1;
    return k;
  }

  double loop_area(const Loop& L)
  {
    const std::size_t n = L.size() / 2;
    double a = 0.;
    for(std::size_t i = 0; i < n; ++i)
      {
        const std::size_t j = (i + 1) % n;
        a += L[2 * i] * L[2 * j + 1] - L[2 * j] * L[2 * i + 1];
      }
    return std::abs(a) * 0.5;
  }

  // Analyze the loop set on the N-grid.
  // Fills `offending` (loop indices in any bad cell or sub-cell) and returns
  // true if the set is ADMISSIBLE (no bad cells, no sub-cell loops).
  bool analyze(const Loops& loops, int N, double lo, double h,
               std::set<int>& offending)
  {
    offending.clear();
    const std::size_t NC = static_cast<std::size_t>(N) * N;
    std::vector<int> cross(NC, 0);
    std::vector<std::set<int>> cellLoops(NC);

    auto touch = [&](int i, int j, int loop) {
      if(i >= 0 && i < N && j >= 0 && j < N)
        { cross[static_cast<std::size_t>(j) * N + i] += 1;
          cellLoops[static_cast<std::size_t>(j) * N + i].insert(loop); }
    };

    std::set<int> subcell;
    for(int li = 0; li < static_cast<int>(loops.size()); ++li)
      {
        const Loop& L = loops[li];
        const std::size_t np = L.size() / 2;
        if(np < 3) { subcell.insert(li); continue; }
        bool crossed = false;
        for(std::size_t s = 0; s < np; ++s)
          {
            const double ax = L[2 * s], ay = L[2 * s + 1];
            const std::size_t t = (s + 1) % np;
            const double bx = L[2 * t], by = L[2 * t + 1];
            if(bx != ax)
              {
                double x0 = ax, x1 = bx, y0 = ay, y1 = by;
                if(x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); }
                for(int i = static_cast<int>(std::ceil((x0 - lo) / h));
                        i <= static_cast<int>(std::floor((x1 - lo) / h)); ++i)
                  {
                    if(i <= 0 || i >= N) continue;
                    const double xi = lo + i * h, u = (xi - x0) / (x1 - x0);
                    if(u <= 0. || u >= 1.) continue;
                    const int jc = cell_index(y0 + u * (y1 - y0), lo, h, N);
                    touch(i - 1, jc, li); touch(i, jc, li); crossed = true;
                  }
              }
            if(by != ay)
              {
                double x0 = ax, x1 = bx, y0 = ay, y1 = by;
                if(y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
                for(int j = static_cast<int>(std::ceil((y0 - lo) / h));
                        j <= static_cast<int>(std::floor((y1 - lo) / h)); ++j)
                  {
                    if(j <= 0 || j >= N) continue;
                    const double yj = lo + j * h, u = (yj - y0) / (y1 - y0);
                    if(u <= 0. || u >= 1.) continue;
                    const int ic = cell_index(x0 + u * (x1 - x0), lo, h, N);
                    touch(ic, j - 1, li); touch(ic, j, li); crossed = true;
                  }
              }
          }
        if(!crossed) subcell.insert(li);
      }

    bool bad = !subcell.empty();
    for(int s : subcell) offending.insert(s);
    for(std::size_t c = 0; c < NC; ++c)
      if(cross[c] > 2)
        { bad = true; for(int li : cellLoops[c]) offending.insert(li); }

    return !bad;
  }

  void write_loops(const Loops& loops, const std::string& path)
  {
    fs::path out(path);
    if(out.has_parent_path()) fs::create_directories(out.parent_path());
    std::ofstream f(path);
    f.precision(9);
    for(std::size_t li = 0; li < loops.size(); ++li)
      {
        if(li) f << '\n';
        const Loop& L = loops[li];
        for(std::size_t k = 0; k < L.size() / 2; ++k)
          f << L[2 * k] << '\t' << L[2 * k + 1] << '\n';
      }
  }
} // namespace


int main(int argc, char** argv)
{
  CLI::App app{"Trim a geometry to admissibility at N by removing smallest offending loops"};
  app.footer("Sample usage:\n  ./trim_geometry -g in.dat -n 128 -o out.dat");

  std::string in_file, out_file;
  int    N  = 128;
  double lo = -1.0, hi = 1.0;

  app.add_option("-g", in_file, "input geometry .dat")->required()->check(CLI::ExistingFile);
  app.add_option("-o", out_file, "output trimmed geometry .dat (written on success)")->required();
  app.add_option("-n", N, "cells across each direction")->required()->check(CLI::PositiveNumber);
  app.add_option("--lo", lo, "domain lower corner");
  app.add_option("--hi", hi, "domain upper corner");

  CLI11_PARSE(app, argc, argv);
  if(hi <= lo) { std::cerr << "error: --hi must exceed --lo" << std::endl; return 2; }
  const double h = (hi - lo) / static_cast<double>(N);

  Loops loops = vm::tutorial::read_polygon_loops(in_file);
  int removed = 0;

  while(true)
    {
      std::set<int> offending;
      if(analyze(loops, N, lo, h, offending))
        {
          write_loops(loops, out_file);
          std::cout << "PASS " << in_file << " removed=" << removed
                    << " loops_left=" << loops.size() << std::endl;
          return 0;
        }
      if(loops.empty()) break;

      // protect the current largest loop (the main body)
      int largest = 0; double amax = -1.;
      for(int i = 0; i < static_cast<int>(loops.size()); ++i)
        { const double a = loop_area(loops[i]); if(a > amax) { amax = a; largest = i; } }

      // smallest-area offending loop that is NOT the main body
      int victim = -1; double amin = 1e300;
      for(int li : offending)
        {
          if(li == largest) continue;
          const double a = loop_area(loops[li]);
          if(a < amin) { amin = a; victim = li; }
        }
      if(victim < 0) break;                         // only the main body offends -> unsalvageable

      loops.erase(loops.begin() + victim);
      ++removed;
    }

  std::cout << "FAIL " << in_file << " unsalvageable removed=" << removed
            << " loops_left=" << loops.size() << std::endl;
  return 1;
}
