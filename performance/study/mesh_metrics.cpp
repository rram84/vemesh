// Sriramajayam

/** \file mesh_metrics.cpp
 * \brief Study tool: per-mesh counts + per-altered-element quality metrics for
 *        the relaxation-vs-agglomeration study (Computational Mechanics paper).
 *
 * Given ONE captured mesh (a per-operation VTK written by `vemesh_app -v op`, or
 * a baseline embedded mesh), this tool emits the mesh-intrinsic quantities the
 * study records, as CSV on stdout. It is a PURE function of the mesh: all study
 * context (geometry, background, driver, workflow, realization, iteration,
 * operation) is passed opaquely via --tag and echoed back verbatim, so the run
 * harness owns the bookkeeping and this tool stays study-structure-agnostic.
 *
 * Two row kinds are emitted, distinguished by a leading field so a single stdout
 * stream carries both (the harness routes G-> globals, F-> perturbed):
 *
 *   G,<tag>,nelems,nverts,n_alt_faces,n_alt_verts
 *   F,<tag>,face_idx,sides,q_stability,q_geom          (one per ALTERED face)
 *
 * The element-quality comparison is recorded ONLY for altered ("perturbed")
 * faces: for well-shaped elements the two metrics barely differ, so the F-rows
 * carry the informative sample. Both metrics come straight from the library
 * (vm::quality::vem_stability_ratio and vm::quality::geom_shape) -- nothing is
 * hand-computed here -- and are evaluated for every altered face irrespective of
 * which one drove the optimization -- quality is a pure function of geometry,
 * whereas the app records only WHICH faces it altered (the `altered` integer
 * field), which is not recoverable from geometry and so is read back from the
 * VTK text. Edge-count statistics over altered elements are derived downstream
 * from the F-rows' `sides` column.
 *
 * The global VEM conditioning number (lambda_max/lambda_2) is NOT computed here;
 * it comes from the MATLAB vem_eig pass, keyed by the same staged filename/tag.
 *
 * \author Ramsharan Rangarajan
 */

#include <vm_io.h>
#include <vm_face_qualities.h>

#include <CLI11.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// Read one integer SCALARS field of exactly `count` values from a VTK file.
// vm::write_vtk emits each scalar field as
//     SCALARS <name> int
//     LOOKUP_TABLE default
//     <v0>
//     ...
// so we scan for the "SCALARS <name> " line, skip the LOOKUP_TABLE line, and
// read `count` integers. Values are returned in file order, which matches the
// face / vertex iteration order of vm::read_vtk on the same file. Returns an
// all-zero vector when the field is absent (a mesh with no alterations).
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
  bool emit_faces = false;   // also stream per-altered-face rows (F,...)

  CLI::App app{"Per-mesh counts + per-altered-element quality (relax-vs-agglomerate study)"};
  app.add_option("-i", meshfile, "input mesh (.vtk captured by vemesh_app)")
    ->required()->check(CLI::ExistingFile);
  app.add_option("--tag", tag,
                 "opaque context string echoed verbatim into every row "
                 "(e.g. geom,bg,driver,workflow,real,iter,op)")->required();
  app.add_flag("--emit-faces", emit_faces,
               "also stream one F-row per ALTERED face (sides,area,q_stability,q_geom)");

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

  // Per-ALTERED-face pass: recompute BOTH metrics and stream F-rows on request.
  std::size_t n_alt_faces = 0;
  std::ostringstream frows;
  std::vector<pmp::Point> coords;
  std::size_t fidx = 0;
  for(auto f : mesh.faces())
  {
    if(fidx < f_altered.size() && f_altered[fidx] != 0)
    {
      ++n_alt_faces;
      if(emit_faces)
      {
        coords.clear();
        for(auto v : mesh.vertices(f)) coords.push_back(mesh.position(v));
        const double qs = vm::quality::vem_stability_ratio(coords);
        const double qg = vm::quality::geom_shape(coords);
        frows << "F," << tag << ',' << fidx << ',' << coords.size() << ','
              << qs << ',' << qg << '\n';
      }
    }
    ++fidx;
  }

  std::cout << "G," << tag << ',' << nelems << ',' << nverts << ','
            << n_alt_faces << ',' << n_alt_verts << '\n';
  if(emit_faces) std::cout << frows.str();
  std::cout.flush();

  return 0;
}
