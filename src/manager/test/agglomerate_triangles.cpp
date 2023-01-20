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

  // checks on filenames: input/output cannot differ just in the extension
  assert(std::string(std::filesystem::path(input_file).stem())!=std::string(std::filesystem::path(output_file).stem()));
  
  // Mesh manager
  const std::string in_file_extension = std::filesystem::path(input_file).extension();
  assert(in_file_extension==".off" || in_file_extension==".OFF");
  vm::Manager mesh_manager(input_file);
  auto& mesh = mesh_manager.get_mesh();

  // write the input mesh in suku's format
  const std::string in_file_stem = std::filesystem::path(input_file).stem();
  vm::write_suku_format(mesh, in_file_stem);
  
  // triangle qualities at input
  auto in_tria_qualities = vm::get_triangle_qualities_set(mesh);
  
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

  // triangle qualities after merging
  auto out_tria_qualities = vm::get_triangle_qualities_set(mesh);
  
  // write
  const std::string out_file_extension = std::filesystem::path(output_file).extension();
  const std::string out_file_stem      = std::filesystem::path(output_file).stem();
  if(out_file_extension==".off" || out_file_extension==".OFF")
    vm::write_off(mesh, output_file);
  else if(out_file_extension.empty()==true) // suku's format
    {
      vm::write_off(mesh, output_file+".OFF");
      vm::write_suku_format(mesh, output_file);
    }

  // triangle qualities in the input mesh
  std::fstream pfile;
  std::string filename = std::string(std::filesystem::path(input_file).stem())+"-tria-quality.dat";
  pfile.open(filename, std::ios::out);
  assert(pfile.good());
  int indx = 0;
  for(auto& it:in_tria_qualities)
    pfile << indx++ << " " << it << std::endl;
  pfile.close();

  // triangle qualities in the output mesh
  filename = std::string(std::filesystem::path(output_file).stem())+"-tria-quality.dat";
  pfile.open(filename, std::ios::out);
  assert(pfile.good());
  indx = 0;
  for(auto& it:out_tria_qualities)
    pfile << indx++ << " " << it << std::endl;
  pfile.close();
  
  // done
}
	 
