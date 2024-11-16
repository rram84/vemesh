// Sriramajayam

#include <vm_io.h>
#include <fstream>
#include <cassert>
#include <filesystem>
#include <sstream>

namespace vm
{
  void write_dat(const pmp::SurfaceMesh &mesh, const std::string filename)
  {
    const std::string extension = std::filesystem::path(filename).extension();
    assert(extension==".dat");

    std::fstream pfile;
    pfile.open(filename, std::ios::out);
    assert(pfile.good());
    
    auto f_circulator = mesh.faces();
    for(auto f:f_circulator)
      {
	auto v_circulator = mesh.vertices(f);
	for(auto v:v_circulator)
	  {
	    const auto& X = mesh.position(v);
	    pfile << X[0] << " " << X[1] << std::endl;
	  }
	// repeat the first vertex
	for(auto v:v_circulator)
	  {
	    const auto& X = mesh.position(v);
	    pfile << X[0] << " " << X[1] << std::endl;
	    break;
	  }
	
	pfile << std::endl;
      }
    pfile.close();
    return;    
  }

}
