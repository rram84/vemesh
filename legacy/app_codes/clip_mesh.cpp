// Sriramajayam

#include <vm_test_mesh_slicer.h>
#include <vm_io.h>
#include <CLI/CLI.hpp>
#include <filesystem>
#include <cmath>

// Flags
// -c Clip the mesh and retain the negative level set
// -e Embed the zero level set as an interface in the mesh

// Options
// -i input mesh file in off or vtk file format
// -p cartesian coordinates of a point on the zero level set
// -a angle of the cut in degrees
// -t offset tolerance for signed distance values
// -o output mesh file in off or vtk file format

// signed distance function to a line passing through P and inclined at theta to the horizontal
double line_signed_distance(const double* X, const std::vector<double>& P, const double theta) {
  return -std::sin(theta)*(X[0]-P[0]) + std::cos(theta)*(X[1]-P[1]);
}

int main(int argc, char** argv) {

  // Command line options
  CLI::App app;
  app.footer("Clip a mesh with a straight line passing through a given point and at a given orientation. \n \
              Sample usage: ./clip_mesh -c -p 1 0.001 -a 12.0 -t 0.0001 -i mesh.off -o mesh-cut.off");

  // flags
  bool flag1 = false;
  bool flag2 = false;
  app.add_flag("-c", flag1, "Clip the mesh with a straight line");
  app.add_flag("-e", flag2, "Embed a straight interface in the mesh");

  // options
  std::string in_meshfile, out_meshfile;
  std::vector<double> point(2);
  double angle, sdtol;
  app.add_option("-i", in_meshfile, "input mesh file in off or vtk format")->required()->check(CLI::ExistingFile);
  app.add_option("-o", out_meshfile, "output mesh file in off or vtk format")->required();
  app.add_option("-a", angle, "slicing angle in degrees")->required();
  app.add_option("-p", point, "point on the slicing segment")->required()->expected(2);
  app.add_option("-t", sdtol, "tolerance for signed distances as nodes")->required();

  // parse
  CLI11_PARSE(app, argc, argv);
  assert( (flag1 || flag2)==true && "Pick one of the options -c to clip and -e to embed");
  assert( (flag1 && flag2)==false && "Pick one of the options -c to clip and -e to embed");
  assert(sdtol>0.);
  
  // read the input mesh
  std::string ext = std::filesystem::path(in_meshfile).extension();
  assert(ext==".off" || ext==".OFF" || ext==".vtk");
  pmp::SurfaceMesh mesh;
  if(ext==".off" || ext==".OFF")
    vm::read_off(in_meshfile, mesh);
  else
    vm::read_vtk(in_meshfile, mesh);

  // create the clipping line
  vm::test::LevelSetFunction_t sdfunc = std::bind(line_signed_distance, std::placeholders::_1, point, angle*M_PI/180.);

  // adjust node positions away from the zero level set
  // vm::test::adjust_mesh_nodes(mesh, sdtol, 1.1*sdtol, sdfunc);
    
  // clip/embed
  if(flag1==true)
    vm::test::clip_mesh(mesh, sdtol, sdfunc, 1, 1); // shifting tolerance, material id, boundary node id
  else
    vm::test::embed_interface(mesh, sdtol, sdfunc, {1,2}, 1); // shifting tolerance, {mat1, mat2}, boundary node id

  // save
  ext = std::filesystem::path(out_meshfile).extension();
  assert(ext==".off" || ext==".OFF" || ext==".vtk");
  if(ext==".off" || ext==".OFF")
    vm::write_off(mesh, out_meshfile);
  else
    vm::write_vtk(mesh, out_meshfile);

  // done
}
