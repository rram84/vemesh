// Sriramajayam

// generate a structured quadrilateral grid over [0,1]^2

// Required options:
// --nx: Number of nodes along the x-direction
// --ny: Number of nodes along the y-direction
// -o  : Output mesh file name in off format, does not overwrite. Also writes a file in vtk format with element qualities

#include <vm_SpecialMeshes.h>
#include <vm_face_quality.h>
#include <vm_io.h>
#include <CLI/CLI.hpp>
#include <set>


int main(int argc, char* argv[])
{
  // Command line options
  CLI::App app;

  // Options
  int nx=0, ny=0;
  app.add_option("--nx", nx, "#nodes along x")->required();
  app.add_option("--ny", ny, "#nodes along y")->required();

  // mesh file
  std::string meshfile;
  app.add_option("-o", meshfile, "output mesh file with OFF extension")->required()->check(!CLI::ExistingFile);

  // parse
  CLI11_PARSE(app, argc, argv);
  assert(nx>=2 && ny>=2);
  
  // construct quad mesh
  const double left_cnr[] = {0.,0.};
  const double hx = 1./static_cast<double>(nx-1);
  const double hy = 1./static_cast<double>(ny-1);
  auto mesh = vm::create_rect_mesh(left_cnr, hx, nx, hy, ny);
  vm::write_off(mesh, meshfile);

  // print mesh with quality in vtk format
  const std::string vtk_filename = std::string(std::filesystem::path(meshfile).stem())+".vtk";
  vm::write_vtk_with_cell_data(mesh, vm::compute_stiffness_based_mesh_face_quality, vtk_filename);
}
  
