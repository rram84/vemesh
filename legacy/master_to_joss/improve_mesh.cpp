// Sriramajayam

#include <vm_Manager.h>
#include <vm_io.h>
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
  app.add_option("-a", eps_degrees, "angle tolerance in degrees to merge triangles")->required()->expected(0.,60.);
  int num_samples;
  app.add_option("-s", num_samples, "number of location samples to condition for relaxation")->required();
  int num_iter;
  app.add_option("-n", num_iter, "number of merge/smooth iterations")->required();
  std::string output_file;
  app.add_option("-o", output_file, "output mesh file")->required();

  // parse options
  CLI11_PARSE(app, argc, argv);

  // checks on filenames:
  // input and output filenames should have .OFF or .off extension
  const std::string in_ext   = std::filesystem::path(input_file).extension();
  const std::string in_stem  = std::filesystem::path(input_file).stem();
  const std::string out_ext  = std::filesystem::path(output_file).extension();
  assert(in_ext==".OFF"  || in_ext==".off");
  assert(out_ext.empty());

  // mesh manager
  vm::Manager manager(input_file);
  auto& mesh = manager.get_mesh();

  // write the input file in suku's format
  vm::write_suku_format(mesh, in_stem);

  // triangle merging & relaxation iterations 
  for(int iter=0; iter<num_iter; ++iter)
    {
      std::cout << std::endl << "Iteration " << iter << ": " << std::endl;

      // merge triangles
      int nmerged = manager.agglomerate_triangles(eps_degrees);
      std::cout << "merged " << nmerged << " triangles " << std::endl;
      
      // move vertices
      int nmoved = manager.move_all_vertices(num_samples);
      std::cout << "moved " << nmoved << " vertices " << std::endl;
    }
  
  // write
  vm::write_off(mesh, output_file+".OFF");
  vm::write_suku_format(mesh, output_file);
}
