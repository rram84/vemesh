// Sriramajayam

// generate a random delaunay triangulation over [0,1]^2

// Required options:
// -n: Number of random vertices to generate
// -m: Output mesh file name in off format, does not overwrite

#include <vm_io.h>
#include <vm_SpecialMeshes.h>
#include <CLI/CLI.hpp>

int main(int argc, char** argv)
{
  // Either provide the number of sample points or the input mesh file
  CLI::App app;

  // Number of random points to generate over the interval [0,1] x [0,1]
  int num_points;
  app.add_option("-n", num_points, "number of random points over unit square")->required()->check(CLI::PositiveNumber);

  // mesh file
  std::string meshfile;
  app.add_option("-m", meshfile, "output mesh file")->required()->check(!CLI::ExistingFile);
  
  // parse
  CLI11_PARSE(app, argc, argv);
  const std::string ext = std::filesystem::path(meshfile).extension();
  assert((ext==".off" || ext==".OFF") && "Expected mesh file name in OFF format");
  
  // mesh
  const double bot_left_cnr[]  = {0.,0.};
  const double top_right_cnr[] = {1.,1.};
  auto mesh = vm::create_random_delaunay(bot_left_cnr, top_right_cnr, num_points);
  vm::write_off(mesh, meshfile);

  // done
}
