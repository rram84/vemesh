// Sriramajayam

/** \file mesh_metrics.cpp
 * \brief Study tool: per-mesh counts + aggregated per-altered-element quality
 *        statistics for the relaxation-vs-agglomeration study (Comp. Mech. paper).
 *
 * Given ONE captured mesh (a per-operation VTK written by `vemesh_app -v op`, or
 * a baseline embedded mesh), this tool emits, as CSV on stdout, the study's
 * mesh-intrinsic quantities. It is a PURE function of the mesh: all study context
 * (geometry, background, driver, workflow, realization, operation) is passed
 * opaquely via --tag and echoed back verbatim, so the run harness owns the
 * bookkeeping and this tool stays study-structure-agnostic.
 *
 * Two row kinds, distinguished by a leading field so one stdout stream carries
 * both (the harness routes them to different CSVs):
 *
 *   G,<tag>,nelems,nverts,n_alt_faces,n_alt_verts
 *   H,<tag>,n_alt,qs_min,qs_max,qg_min,qg_max,qs_sum,qg_sum,qs_sumsq,qg_sumsq,
 *          qsqg_sum, <qs_hist x QNB>, <qg_hist x QNB>, <sides_hist x SNB>
 *
 * The H row is emitted only when the mesh has altered ("perturbed") faces (a flag
 * gates it). It records the DISTRIBUTION of the two quality metrics over the
 * altered set -- not the individual element values -- as FIXED-bin histograms
 * plus extremes and running sums. Fixed bins make everything additive: pooling
 * across realizations / geometries is a column-wise sum, and quantiles / violins
 * are read off the pooled histogram; the running sums give exact pooled means,
 * variances and the stability-vs-geom Pearson correlation. Storing histograms
 * rather than every element value keeps the study output small.
 *
 * The element-quality comparison is over altered faces only: for well-shaped
 * elements the two metrics barely differ, so the altered set is the informative
 * sample. Both metrics come straight from the library (vm::quality::
 * vem_stability_ratio and vm::quality::geom_shape) -- nothing is hand-computed --
 * evaluated for every altered face irrespective of which drove the optimization;
 * the app records only WHICH faces it altered (the `altered` integer field), read
 * back from the VTK text since read_vtk skips it.
 *
 * Bins (documented for the CSV header): q_stability and q_geom are in [0,1] with
 * QNB uniform bins (bin b covers [b/QNB,(b+1)/QNB)); sides bin b covers b+SMIN
 * sides for b<SNB-1 and ">= SMIN+SNB-1" for the last (overflow) bin.
 *
 * The global VEM conditioning is computed separately (mesh_conditioning); it is
 * NOT recomputed here.
 *
 * \author Ramsharan Rangarajan
 */

#include <vm_io.h>
#include <vm_face_qualities.h>

#include <CLI11.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr int QNB  = 1000; // quality histogram bins over [0,1] (fine-grained)
constexpr int SMIN = 3;    // first sides bin = triangles
constexpr int SNB  = 48;   // sides bins: b -> (SMIN+b) sides, last bin = overflow (>= SMIN+SNB-1 = 50)

int q_bin(double v)
{
  int b = static_cast<int>(std::floor(v * QNB));
  if(b < 0) b = 0;
  if(b >= QNB) b = QNB - 1;
  return b;
}

int s_bin(int sides)
{
  int b = sides - SMIN;
  if(b < 0) b = 0;
  if(b >= SNB) b = SNB - 1;
  return b;
}

// Read one integer SCALARS field of exactly `count` values from a VTK file.
// vm::write_vtk emits each scalar field as "SCALARS <name> int / LOOKUP_TABLE
// default / v0 / v1 / ...". Values come back in file order, matching the face /
// vertex iteration order of vm::read_vtk on the same file. Absent field -> zeros.
std::vector<int> read_int_field(const std::string& filename,
                                const std::string& field,
                                std::size_t count)
{
  std::ifstream file(filename);
  if(!file.is_open() || !file.good())
    throw std::runtime_error("read_int_field: could not open " + filename);

  const std::string key = "SCALARS " + field + " ";
  std::string line;
  bool found = false;
  while(std::getline(file, line))
    if(line.find(key) != std::string::npos) { found = true; break; }

  if(!found)
    return std::vector<int>(count, 0);

  std::getline(file, line);              // skip "LOOKUP_TABLE default"

  std::vector<int> vals(count);
  for(std::size_t i=0; i<count; ++i)
    if(!(file >> vals[i]))
      throw std::runtime_error("read_int_field: short read for '" + field +
                               "' in " + filename);
  return vals;
}

} // namespace

int main(int argc, char** argv)
{
  std::string meshfile, tag;
  bool altered_stats = false;   // also emit the H row (altered-set histograms)

  CLI::App app{"Per-mesh counts + aggregated altered-element quality (relax-vs-agglomerate study)"};
  app.add_option("-i", meshfile, "input mesh (.vtk captured by vemesh_app)")
    ->required()->check(CLI::ExistingFile);
  app.add_option("--tag", tag,
                 "opaque context string echoed verbatim into every row")->required();
  app.add_flag("--altered-stats", altered_stats,
               "also emit the H row: histograms + extremes + sums over ALTERED faces");

  CLI11_PARSE(app, argc, argv);

  // Geometry (face/vertex order == file order == alteration-array order).
  const pmp::SurfaceMesh mesh = vm::read_vtk(meshfile);
  const std::size_t nelems = mesh.n_faces();
  const std::size_t nverts = mesh.n_vertices();

  // Alteration flags, read back from the VTK (not recoverable from geometry).
  const std::vector<int> f_altered = read_int_field(meshfile, "altered", nelems);
  const std::vector<int> v_altered = read_int_field(meshfile, "vertex_altered", nverts);
  const std::size_t n_alt_verts =
    std::count_if(v_altered.begin(), v_altered.end(), [](int x){ return x != 0; });

  // Per-ALTERED-face pass: accumulate distributions of both metrics.
  std::size_t n_alt_faces = 0;
  double qs_min =  std::numeric_limits<double>::infinity(), qs_max = -1.0, qs_sum = 0.0, qs_sq = 0.0;
  double qg_min =  std::numeric_limits<double>::infinity(), qg_max = -1.0, qg_sum = 0.0, qg_sq = 0.0;
  double qsqg_sum = 0.0;
  std::array<long, QNB> qs_hist{}; qs_hist.fill(0);
  std::array<long, QNB> qg_hist{}; qg_hist.fill(0);
  std::array<long, SNB> s_hist{};  s_hist.fill(0);

  std::vector<pmp::Point> coords;
  std::size_t fidx = 0;
  for(auto f : mesh.faces())
  {
    if(fidx < f_altered.size() && f_altered[fidx] != 0)
    {
      ++n_alt_faces;
      coords.clear();
      for(auto v : mesh.vertices(f)) coords.push_back(mesh.position(v));
      const double qs = vm::quality::vem_stability_ratio(coords);
      const double qg = vm::quality::geom_shape(coords);
      const int    ns = static_cast<int>(coords.size());

      qs_min = std::min(qs_min, qs); qs_max = std::max(qs_max, qs); qs_sum += qs; qs_sq += qs*qs;
      qg_min = std::min(qg_min, qg); qg_max = std::max(qg_max, qg); qg_sum += qg; qg_sq += qg*qg;
      qsqg_sum += qs*qg;
      ++qs_hist[q_bin(qs)];
      ++qg_hist[q_bin(qg)];
      ++s_hist[s_bin(ns)];
    }
    ++fidx;
  }

  std::cout.setf(std::ios::scientific);
  std::cout.precision(10);
  std::cout << "G," << tag << ',' << nelems << ',' << nverts << ','
            << n_alt_faces << ',' << n_alt_verts << '\n';

  if(altered_stats && n_alt_faces > 0)
  {
    std::cout << "H," << tag << ',' << n_alt_faces << ','
              << qs_min << ',' << qs_max << ',' << qg_min << ',' << qg_max << ','
              << qs_sum << ',' << qg_sum << ',' << qs_sq << ',' << qg_sq << ',' << qsqg_sum;
    for(long c : qs_hist) std::cout << ',' << c;
    for(long c : qg_hist) std::cout << ',' << c;
    for(long c : s_hist)  std::cout << ',' << c;
    std::cout << '\n';
  }
  std::cout.flush();
  return 0;
}
