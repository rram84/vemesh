// Sriramajayam

#include <vm_test_rectangle_mesh.h>
#include <vm_io.h>
#include <CLI/CLI.hpp>
#include <filesystem>

// Options
// -n Number of square-shaped elements per side of the unit square
// -o output mesh filename

int main(int argc, char** argv) {

  // Command line options
  CLI::App app;
  app.footer("Generate a structured quad mesh over [-1,1]^2. \n\
              Sample usage: ./gen_square_mesh -n 10 -o sqmesh.OFF");

  // Options
  int n;
  std::string meshfile;
  app.add_option("-n", n, "number of cells per side")->required();
  app.add_option("-o", meshfile, "output mesh file with OFF or vtk extension")->required();

  // parse
  CLI11_PARSE(app, argc, argv);
  
  // create a rectangular mesh
  const double left_cnr[] = {-1.,-1.};
  const int mat_id = 1;
  const double h = 2.0/static_cast<double>(n);
  pmp::SurfaceMesh sq_mesh = vm::test::create_rect_mesh(left_cnr, h, n+1, h, n+1, 1);
  const std::string ext = std::filesystem::path(meshfile).extension();
  assert(ext==".off" || ext==".OFF" || ext==".vtk");
  if(ext==".off" || ext==".OFF")
    vm::write_off(sq_mesh, meshfile);
  else
    vm::write_vtk(sq_mesh, meshfile);

  // done
}

