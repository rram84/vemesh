// Sriramajayam

// Convert between mesh formats
// (vtk, off) -> (off, vtk, node/ele)

#include <vm_io.h>
#include <CLI/CLI.hpp>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char** argv)
{
  // Command line options
  CLI::App app;

  // Input mesh file(s)
  std::vector<std::string> in;
  app.add_option("-i", in, "input mesh files in .off/.OFF or .vtk format")->required();

  // Output mesh file(s)
  std::string out;
  app.add_option("-o", out, "output mesh file in .off or .vtk format")->required()->check(!CLI::ExistingFile);

  // parse
  CLI11_PARSE(app, argc, argv);

  // check in/out files and extensions, and convert
  const int nin = static_cast<int>(in.size());
  std::vector<std::string> in_ext{};
  assert(nin==1 || nin==2);
  for(auto& f:in)
    {
      assert(fs::exists(f));
      in_ext.push_back(fs::path(f).extension());
    }
  const std::string out_ext = fs::path(out).extension();
  
  if(nin==1)   // .off or .OFF -> vtk
    {
      assert((in_ext[0]==".off" || in_ext[0]==".OFF") && "Expected .off/.OFF input file extension");
      assert(out_ext==".vtk" && "Unexpected output file mesh format, should be vtk");

      pmp::SurfaceMesh mesh;
      vm::read_off(in[0], mesh);
      if(out_ext==".vtk")
	vm::write_vtk(mesh, out);
      else
	assert(false && "Unexpected file format");
    }
  else       // .node, .ele -> off, vtk
    {
      assert(((in_ext[0]==".node" && in_ext[1]==".ele") || (in_ext[0]==".ele" && in_ext[1]==".node"))
	     && "Expected .node,.ele input file extension");
      assert((out_ext==".off" || out_ext==".OFF" || out_ext==".vtk") &&
	     "Unexpected output file mesh format, should be OFF or vtk");

      pmp::SurfaceMesh mesh;
      if(in_ext[0]==".node")
	vm::read_triangles(in[0], in[1], mesh);
      else
	vm::read_triangles(in[1], in[0], mesh);

      if(out_ext==".off" || out_ext==".OFF")
	vm::write_off(mesh, out);
      else if(out_ext==".vtk")
	vm::write_vtk(mesh, out);
      else
	assert(false && "Unexpected scenario");
    }
  
  // done
}

