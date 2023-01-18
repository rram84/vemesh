// Sriramajayam

#include <vm_Manager.h>
#include <vm_io.h>
#include <fstream>
#include <filesystem>
#include <CLI/CLI.hpp>


int main(int argc, char** argv)
{
  // Command line options
  CLI::App app;

  // Options
  std::string input_file;
  app.add_option("-i", input_file, "input mesh file in OFF format")->required()->check(CLI::ExistingFile);
  double eps_degrees;
  app.add_option("-a", eps_degrees, "angle tolerance (in degrees) to merge triangles")->required()->expected(0.,60.);
  std::string output_file;
  app.add_option("-o", output_file, "output mesh file")->required()->check(!CLI::ExistingFile);

  // parse options
  CLI11_PARSE(app, argc, argv);

  // Mesh manager
  const std::string in_file_extension = std::filesystem::path(input_file).extension();
  assert(in_file_extension==".off" || in_file_extension==".OFF");
  vm::Manager mesh_manager(input_file);

  // iteratively merge triangles
  int iter = 0;
  while(true)
    {
      std::cout << "Iteration " << ++iter << " : " << std::flush;
      int nmerged = mesh_manager.agglomerate_triangles(eps_degrees);
      std::cout << "merged " << nmerged << " triangles" << std::endl;
      if(nmerged==0)
	break;
    }

  // write
  auto& mesh = mesh_manager.get_mesh();
  const std::string out_file_extension = std::filesystem::path(output_file).extension();
  const std::string out_file_stem      = std::filesystem::path(output_file).stem();
  if(out_file_extension==".off" || out_file_extension==".OFF")
    vm::write_off(mesh, output_file);
  else if(out_file_extension.empty()==true) // suku's format
    {
      vm::write_off(mesh, output_file+".OFF");
      vm::write_suku_format(mesh, output_file);
    }

  // done
}
	 
