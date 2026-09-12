// Sriramajayam

/** \file coarsen_geometry.cpp
 * \brief Study tool: mesh-INDEPENDENT geometry coarsening. Removes features finer
 *        than a length scale delta (~ cell size) by morphological close-then-open,
 *        so the frozen result embeds cleanly at the target resolution and stays
 *        valid under background-mesh perturbation.
 * \ingroup performance_examples
 * \author Ramsharan Rangarajan
 */

// The embedder can only represent interface features coarser than a cell. We
// therefore coarsen each geometry ONCE, to a fixed length scale delta, with no
// reference to the mesh or grid — so the same frozen curve is used for every
// perturbed realization.
//
// Method: assemble the loops into a filled region (multipolygon with holes, via
// even-odd nesting), then apply morphological
//   close  = dilate(+delta) then erode(-delta)   -> fills gaps/necks < ~2 delta
//            (merges two branches sharing a cell; fills a self-neck pinch)
//   open   = erode(-delta) then dilate(+delta)    -> removes slivers/protrusions
//            (thin appendages) < ~2 delta
// in that order (close-then-open). The result has no feature finer than ~2 delta,
// so with delta ~ h it is admissible at N and robust to <=0.15h node jitter.
//
// Output: writes the coarsened rings (outer + holes) as blank-line-separated
// loops (read_polygon_loops format) to -o. One status line + exit code
// (0 = ok, 1 = geometry vanished under coarsening -> drop).
//
// Usage:
//   coarsen_geometry -g in.dat -o out.dat [--delta 0.015625] [--ppc 8]

#include <vm_tutorial_polygonSDF.h>          // vm::tutorial::read_polygon_loops

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>

#include <CLI11.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace bg = boost::geometry;
namespace fs = std::filesystem;

using Pt    = bg::model::d2::point_xy<double>;
using Poly  = bg::model::polygon<Pt>;         // default: clockwise, closed
using MPoly = bg::model::multi_polygon<Poly>;

namespace
{
  Poly ring_to_poly(const std::vector<double>& L)
  {
    Poly p;
    for(std::size_t k = 0; k < L.size() / 2; ++k)
      bg::append(p.outer(), Pt(L[2 * k], L[2 * k + 1]));
    bg::correct(p);
    return p;
  }

  // Build the even-odd filled region as a multipolygon with holes.
  MPoly assemble_region(const std::vector<std::vector<double>>& loops)
  {
    const int n = static_cast<int>(loops.size());
    std::vector<Poly> polys(n);
    for(int i = 0; i < n; ++i) polys[i] = ring_to_poly(loops[i]);

    // depth[i] = number of loops strictly containing loop i (even=outer, odd=hole)
    std::vector<int> depth(n, 0);
    for(int i = 0; i < n; ++i)
      for(int j = 0; j < n; ++j)
        if(i != j && bg::within(polys[i], polys[j])) ++depth[i];

    // immediate parent = the containing loop with the greatest depth (smallest container)
    std::vector<int> parent(n, -1);
    for(int i = 0; i < n; ++i)
      for(int j = 0; j < n; ++j)
        if(i != j && bg::within(polys[i], polys[j]))
          if(parent[i] < 0 || depth[j] > depth[parent[i]]) parent[i] = j;

    // each even-depth loop is an outer ring; its odd-depth children are holes
    MPoly region;
    for(int i = 0; i < n; ++i)
      {
        if(depth[i] % 2 != 0) continue;               // holes handled via their parent
        Poly p;
        p.outer() = polys[i].outer();
        for(int k = 0; k < n; ++k)
          if(parent[k] == i)
            p.inners().push_back(polys[k].outer());
        bg::correct(p);
        region.push_back(std::move(p));
      }
    return region;
  }

  MPoly buffer_by(const MPoly& in, double d, int ppc)
  {
    bg::strategy::buffer::distance_symmetric<double> dist(d);
    bg::strategy::buffer::side_straight side;
    bg::strategy::buffer::join_round   join(ppc);
    bg::strategy::buffer::end_round    end(ppc);
    bg::strategy::buffer::point_circle circle(ppc);
    MPoly out;
    bg::buffer(in, out, dist, side, join, end, circle);
    return out;
  }

  void write_region(const MPoly& region, const std::string& path)
  {
    fs::path out(path);
    if(out.has_parent_path()) fs::create_directories(out.parent_path());
    std::ofstream f(path);
    f.precision(9);
    bool first = true;
    auto write_ring = [&](const Poly::ring_type& r) {
      if(r.size() < 4) return;                          // <3 distinct pts
      if(!first) f << '\n';
      first = false;
      // ring is closed (front==back); drop the duplicated last point
      for(std::size_t k = 0; k + 1 < r.size(); ++k)
        f << bg::get<0>(r[k]) << '\t' << bg::get<1>(r[k]) << '\n';
    };
    for(const auto& p : region)
      {
        write_ring(p.outer());
        for(const auto& h : p.inners()) write_ring(h);
      }
  }
} // namespace


int main(int argc, char** argv)
{
  CLI::App app{"Mesh-independent geometry coarsening (morphological close-then-open)"};
  app.footer("Sample usage:\n  ./coarsen_geometry -g in.dat -o out.dat --delta 0.0156");

  std::string in_file, out_file, mode = "close";
  double delta = 2.0 / 128.0;   // ~ cell size h at N=128 over [-1,1]
  int    ppc   = 8;             // points per circle (corner rounding fidelity)

  app.add_option("-g", in_file, "input geometry .dat")->required()->check(CLI::ExistingFile);
  app.add_option("-o", out_file, "output coarsened geometry .dat")->required();
  app.add_option("--delta", delta, "coarsening length scale")->check(CLI::PositiveNumber);
  app.add_option("--mode", mode, "close (merge nearby loops; fills gaps, no erosion) | "
                 "open (remove thin features) | closeopen (both)")
     ->check(CLI::IsMember({"close", "open", "closeopen"}));
  app.add_option("--ppc", ppc, "points per circle for round joins")->check(CLI::PositiveNumber);

  CLI11_PARSE(app, argc, argv);

  const auto loops = vm::tutorial::read_polygon_loops(in_file);

  MPoly region, result;
  double in_area = 0., out_area = 0.;
  try
    {
      region  = assemble_region(loops);
      in_area = bg::area(region);
      result  = region;
      // close = dilate(+d) then erode(-d): merges nearby loops, fills gaps < ~2d,
      //         never erodes features away (right tool for inter-component fixes).
      if(mode == "close" || mode == "closeopen")
        {
          MPoly a = buffer_by(result, +delta, ppc);
          result  = buffer_by(a,      -delta, ppc);
        }
      // open = erode(-d) then dilate(+d): removes protrusions/slivers < ~2d.
      if(mode == "open" || mode == "closeopen")
        {
          MPoly b = buffer_by(result, -delta, ppc);
          result  = buffer_by(b,      +delta, ppc);
        }
      out_area = bg::area(result);
    }
  catch(const std::exception& e)
    {
      std::cout << "FAIL " << in_file << " exception:" << e.what() << std::endl;
      return 1;
    }

  if(result.empty() || out_area <= 0.)
    {
      std::cout << "DROP " << in_file << " vanished (delta too large for this shape)" << std::endl;
      return 1;
    }

  // shape-change metric: Jaccard distance 1 - area(orig ∩ coarse)/area(orig ∪ coarse)
  // (0 = identical, ->1 = drastically changed). Lets the caller drop over-coarsened shapes.
  double change = 1.0;
  try
    {
      MPoly inter, uni;
      bg::intersection(region, result, inter);
      bg::union_(region, result, uni);
      const double ai = bg::area(inter), au = bg::area(uni);
      if(au > 0.) change = 1.0 - ai / au;
    }
  catch(const std::exception&) { change = 1.0; }

  // count output loops (rings)
  int out_loops = 0;
  for(const auto& p : result) { out_loops += 1 + static_cast<int>(p.inners().size()); }

  write_region(result, out_file);
  std::cout << "OK " << in_file
            << " in_loops=" << loops.size() << " out_loops=" << out_loops
            << " in_area=" << in_area << " out_area=" << out_area
            << " change=" << change << std::endl;
  return 0;
}
