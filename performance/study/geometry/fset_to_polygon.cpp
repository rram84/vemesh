// Sriramajayam

/** \file fset_to_polygon.cpp
 * \brief Study tool: oversample piecewise-Bezier feature sets (.fset) into
 *        polygonal (.dat) interfaces for VEMesh's embedded-geometry study.
 * \ingroup performance_examples
 * \author Ramsharan Rangarajan
 */

// Oversample a piecewise-Bezier feature set (.fset) into a polygonal
// representation (.dat) consumable by vm::tutorial::read_polygon_loops.
//
// .fset format (one header comment + a curve count, then one record per segment):
//   #Format: curve_id, npoles, poles, nweights, weights
//   <ncurves>
//   <curve_id> <npoles> <pole_x0 pole_y0 ...> <nweights> <w0 ...>
// with
//   npoles==2 -> line segment              (nweights==0)
//   npoles==3 -> rational quadratic Bezier (nweights==3; exact conics/arcs)
//   npoles==4 -> cubic Bezier              (nweights==0)
//
// Output .dat: one "x y" boundary sample per line, ONE LOOP PER BLOCK, blocks
// separated by a blank line (matches vm::tutorial::read_polygon_loops). A single
// loop is a plain list. Segment endpoints are always retained, so every corner
// lands on a vertex. Loops are assembled by chaining segments on shared
// endpoints. This study uses CLOSED loops only: a shape with any dangling
// endpoint (an open component) is skipped unless --keep-open is given.
//
// Usage:
//   fset_to_polygon -i in.fset -o out.dat [--curve-samples 8] [--line-samples 1]
//   fset_to_polygon --batch -i in_dir -o out_dir [...]   # closed-only by default

#include <CLI11.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace
{
  using Point = std::array<double, 2>;
  using Key   = std::pair<std::int64_t, std::int64_t>;   // quantized endpoint identity

  constexpr double KEY_SCALE = 1e6;                       // ~1e-6 pole precision

  Key key_of(const Point& p)
  {
    return {static_cast<std::int64_t>(std::llround(p[0] * KEY_SCALE)),
            static_cast<std::int64_t>(std::llround(p[1] * KEY_SCALE))};
  }

  struct Segment
  {
    int                 npoles = 0;
    std::vector<double> poles;      // 2*npoles entries
    std::vector<double> weights;    // rational quadratic only
    Point               a{};        // first pole
    Point               b{};        // last pole
  };

  // Evaluate nsamp+1 points along one Bezier segment (endpoints included).
  std::vector<Point> eval_segment(const Segment& s, int nsamp)
  {
    const int np = s.npoles;
    std::vector<Point> P(np);
    for(int i = 0; i < np; ++i) P[i] = {s.poles[2 * i], s.poles[2 * i + 1]};

    std::vector<Point> out;
    out.reserve(nsamp + 1);
    for(int k = 0; k <= nsamp; ++k)
      {
        const double t = static_cast<double>(k) / nsamp;
        Point X{};
        if(np == 2)
          {
            X[0] = (1 - t) * P[0][0] + t * P[1][0];
            X[1] = (1 - t) * P[0][1] + t * P[1][1];
          }
        else if(np == 4)
          {
            const double b0 = (1 - t) * (1 - t) * (1 - t);
            const double b1 = 3 * (1 - t) * (1 - t) * t;
            const double b2 = 3 * (1 - t) * t * t;
            const double b3 = t * t * t;
            X[0] = b0 * P[0][0] + b1 * P[1][0] + b2 * P[2][0] + b3 * P[3][0];
            X[1] = b0 * P[0][1] + b1 * P[1][1] + b2 * P[2][1] + b3 * P[3][1];
          }
        else // np == 3: rational quadratic
          {
            const double w0 = s.weights.size() == 3 ? s.weights[0] : 1.0;
            const double w1 = s.weights.size() == 3 ? s.weights[1] : 1.0;
            const double w2 = s.weights.size() == 3 ? s.weights[2] : 1.0;
            const double b0 = (1 - t) * (1 - t) * w0;
            const double b1 = 2 * (1 - t) * t * w1;
            const double b2 = t * t * w2;
            const double den = b0 + b1 + b2;
            X[0] = (b0 * P[0][0] + b1 * P[1][0] + b2 * P[2][0]) / den;
            X[1] = (b0 * P[0][1] + b1 * P[1][1] + b2 * P[2][1]) / den;
          }
        out.push_back(X);
      }
    return out;
  }

  // Parse a .fset into its segments. Throws std::runtime_error on bad input.
  std::vector<Segment> parse_fset(const std::string& path)
  {
    std::ifstream in(path);
    if(!in.is_open())
      throw std::runtime_error("cannot open " + path);

    std::vector<std::string> records;
    std::string line;
    while(std::getline(in, line))
      {
        // strip a leading '#'-comment line and blanks
        std::size_t p = line.find_first_not_of(" \t\r\n");
        if(p == std::string::npos || line[p] == '#') continue;
        records.push_back(line);
      }
    if(records.empty())
      throw std::runtime_error("empty fset: " + path);

    std::vector<Segment> segs;
    // records[0] is the curve count; the rest are segment records
    for(std::size_t r = 1; r < records.size(); ++r)
      {
        std::istringstream ss(records[r]);
        Segment s;
        int curve_id = 0, nweights = 0;
        if(!(ss >> curve_id >> s.npoles)) continue;
        if(s.npoles != 2 && s.npoles != 3 && s.npoles != 4)
          throw std::runtime_error("unsupported npoles in " + path);
        s.poles.resize(2 * s.npoles);
        for(int i = 0; i < 2 * s.npoles; ++i) ss >> s.poles[i];
        if(ss >> nweights)
          {
            s.weights.resize(nweights);
            for(int i = 0; i < nweights; ++i) ss >> s.weights[i];
          }
        s.a = {s.poles[0], s.poles[1]};
        s.b = {s.poles[2 * s.npoles - 2], s.poles[2 * s.npoles - 1]};
        segs.push_back(std::move(s));
      }
    return segs;
  }

  // Chain segments into ordered closed loops by shared endpoints.
  // Returns loops (no repeated closing point); n_open counts non-closing chains.
  std::vector<std::vector<Point>> assemble_loops(const std::vector<Segment>& segs,
                                                 int curve_samples, int line_samples,
                                                 int& n_open)
  {
    // adjacency: endpoint key -> list of (segment index, oriented forward?)
    std::map<Key, std::vector<std::pair<int, bool>>> adj;
    for(int si = 0; si < static_cast<int>(segs.size()); ++si)
      {
        adj[key_of(segs[si].a)].push_back({si, true});
        adj[key_of(segs[si].b)].push_back({si, false});
      }

    std::vector<char> used(segs.size(), 0);
    std::vector<std::vector<Point>> loops;
    n_open = 0;

    auto seg_points = [&](int si, bool forward) {
      const Segment& s = segs[si];
      const int nsamp = (s.npoles == 2) ? line_samples : curve_samples;
      std::vector<Point> pts = eval_segment(s, nsamp);
      if(!forward) std::reverse(pts.begin(), pts.end());
      return pts;
    };

    for(int start = 0; start < static_cast<int>(segs.size()); ++start)
      {
        if(used[start]) continue;
        std::vector<Point> loop;
        int si = start; bool forward = true; bool closed = false;
        const Key start_node = key_of(segs[start].a);
        std::size_t guard = 0;
        while(si >= 0 && !used[si] && guard <= segs.size())
          {
            ++guard;
            used[si] = 1;
            std::vector<Point> pts = seg_points(si, forward);
            loop.insert(loop.end(), pts.begin(), pts.end() - 1);  // drop shared end node
            const Key end_node = key_of(forward ? segs[si].b : segs[si].a);
            if(end_node == start_node) { closed = true; break; }
            int nxt = -1; bool nxt_fwd = true;
            auto it = adj.find(end_node);
            if(it != adj.end())
              for(const auto& [nsi, na_is_start] : it->second)
                if(!used[nsi]) { nxt = nsi; nxt_fwd = na_is_start; break; }
            if(nxt < 0) break;
            si = nxt; forward = nxt_fwd;
          }
        if(!closed) ++n_open;
        if(loop.size() >= 3) loops.push_back(std::move(loop));
      }
    return loops;
  }

  void write_dat(const std::vector<std::vector<Point>>& loops, const std::string& path)
  {
    fs::path out(path);
    if(out.has_parent_path()) fs::create_directories(out.parent_path());
    std::ofstream f(path);
    if(!f.is_open()) throw std::runtime_error("cannot write " + path);
    f.setf(std::ios::fmtflags(0), std::ios::floatfield);
    f.precision(9);
    for(std::size_t li = 0; li < loops.size(); ++li)
      {
        if(li) f << '\n';                       // blank line separates loops
        for(const auto& p : loops[li]) f << p[0] << '\t' << p[1] << '\n';
      }
  }

  // Returns points written, or -1 if skipped (open component under closed-only).
  // Scale loops (aspect-preserving) so the larger bbox extent == fit, centered at origin.
  void fit_to_box(std::vector<std::vector<Point>>& loops, double fit)
  {
    double minx = 1e300, miny = 1e300, maxx = -1e300, maxy = -1e300;
    for(const auto& l : loops)
      for(const auto& p : l)
        { minx = std::min(minx, p[0]); maxx = std::max(maxx, p[0]);
          miny = std::min(miny, p[1]); maxy = std::max(maxy, p[1]); }
    const double cx = 0.5 * (minx + maxx), cy = 0.5 * (miny + maxy);
    const double ext = std::max(maxx - minx, maxy - miny);
    if(ext <= 0.) return;
    const double s = fit / ext;
    for(auto& l : loops)
      for(auto& p : l) { p[0] = (p[0] - cx) * s; p[1] = (p[1] - cy) * s; }
  }

  long convert(const std::string& in, const std::string& out,
               int curve_samples, int line_samples, bool closed_only, double fit,
               int& n_loops, int& n_open)
  {
    std::vector<Segment> segs = parse_fset(in);
    std::vector<std::vector<Point>> loops = assemble_loops(segs, curve_samples, line_samples, n_open);
    if(closed_only && n_open > 0) return -1;
    if(fit > 0.) fit_to_box(loops, fit);
    n_loops = static_cast<int>(loops.size());
    long npts = 0;
    for(const auto& l : loops) npts += static_cast<long>(l.size());
    write_dat(loops, out);
    return npts;
  }
} // namespace


int main(int argc, char** argv)
{
  CLI::App app{"Oversample .fset splines into polygonal .dat (closed loops)"};
  app.footer("Sample usage:\n"
             "  ./fset_to_polygon -i shape.fset -o shape.dat\n"
             "  ./fset_to_polygon --batch -i fset_dir -o dat_dir");

  std::string in_path, out_path;
  int curve_samples = 8, line_samples = 1;
  bool batch = false, keep_open = false;
  double fit = 0.0;   // if > 0, scale so max(width,height)=fit and center at origin

  app.add_option("-i", in_path, "input .fset file (or directory with --batch)")->required();
  app.add_option("-o", out_path, "output .dat file (or directory with --batch)")->required();
  app.add_flag("--batch", batch, "treat -i/-o as directories");
  app.add_option("--curve-samples", curve_samples, "subdivisions per Bezier segment")->check(CLI::PositiveNumber);
  app.add_option("--line-samples", line_samples, "subdivisions per line segment")->check(CLI::PositiveNumber);
  app.add_option("--fit", fit, "scale each shape (aspect-preserving) so its larger extent = fit, centered at origin (0 = off)")->check(CLI::NonNegativeNumber);
  app.add_flag("--keep-open", keep_open, "do NOT skip shapes with open components");

  CLI11_PARSE(app, argc, argv);
  const bool closed_only = !keep_open;

  if(!batch)
    {
      int nloops = 0, nopen = 0;
      const long npts = convert(in_path, out_path, curve_samples, line_samples,
                                closed_only, fit, nloops, nopen);
      if(npts < 0)
        std::cout << "skipped (open): " << in_path << std::endl;
      else
        std::cout << fs::path(in_path).filename().string() << " -> "
                  << fs::path(out_path).filename().string()
                  << "  loops=" << nloops << " points=" << npts << std::endl;
      return 0;
    }

  fs::create_directories(out_path);
  int n_ok = 0, n_skip = 0, n_total = 0;
  for(const auto& e : fs::directory_iterator(in_path))
    {
      if(!e.is_regular_file() || e.path().extension() != ".fset") continue;
      ++n_total;
      const std::string stem = e.path().stem().string();
      const std::string out  = (fs::path(out_path) / (stem + ".dat")).string();
      try
        {
          int nloops = 0, nopen = 0;
          const long npts = convert(e.path().string(), out, curve_samples, line_samples,
                                    closed_only, fit, nloops, nopen);
          if(npts < 0) ++n_skip; else ++n_ok;
        }
      catch(const std::exception& ex)
        {
          std::cerr << "  ERROR " << e.path().filename().string() << ": " << ex.what() << std::endl;
          ++n_skip;
        }
    }
  std::cout << "converted " << n_ok << ", skipped " << n_skip
            << ", of " << n_total << " .fset files -> " << out_path << std::endl;
  return 0;
}
