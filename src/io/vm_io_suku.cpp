// Sriramajayam

#include <vm_io.h>

namespace vm
{
  // Writes a mesh in Sukumar's format
  void write_suku_format(const pmp::SurfaceMesh& mesh,
			 const std::string filename)
  {
    const std::string extension = std::filesystem::path(filename).extension();
    assert(extension.empty()==true);
    
    // Node file   : X, Y, boundary flag
    // Element file: material id, #edges, #nodes per edge, vertex ids

    // node file
    std::fstream pfile;
    pfile.open(filename+".node", std::ios::out);
    assert(pfile.good());
    const auto& v_circulator = mesh.vertices();
    for(auto v:v_circulator)
      {
	const auto& X = mesh.position(v); 
	bool bd_flag  = mesh.is_boundary(v);
	pfile << X[0] << "  " << X[1] << " " << static_cast<int>(bd_flag) << std::endl;
      }
    pfile.close();
    
    // element file
    const int mat_id = 1;
    pfile.open(filename+".ele", std::ios::out);
    assert(pfile.good());
    const auto& f_circulator = mesh.faces();
    for(auto f:f_circulator)
      {
	pfile << mat_id << " " ;
	auto f_verts = mesh.vertices(f);
	for(auto v:f_verts)
	  pfile << v.idx()+1 << " " ;
	pfile << std::endl;
      }

    pfile.close();

    // done
    return;
  }


  // Write a mesh in Sukumar's format with element ids
  void write_suku_format_with_cell_id(const pmp::SurfaceMesh& mesh, const std::string filename)
  {
    assert(mesh.has_face_property("id")==true);
    auto id_property = mesh.get_face_property<int>("id");

    const std::string extension = std::filesystem::path(filename).extension();
    assert(extension.empty()==true);
    
    // Node file   : X, Y, boundary flag
    // Element file: material id, #edges, #nodes per edge, vertex ids

    // node file
    std::fstream pfile;
    pfile.open(filename+".node", std::ios::out);
    assert(pfile.good());
    const auto& v_circulator = mesh.vertices();
    for(auto v:v_circulator)
      {
	const auto& X = mesh.position(v); 
	bool bd_flag  = mesh.is_boundary(v);
	pfile << X[0] << "  " << X[1] << " " << static_cast<int>(bd_flag) << std::endl;
      }
    pfile.close();
    
    // element file
    pfile.open(filename+".ele", std::ios::out);
    assert(pfile.good());
    const auto& f_circulator = mesh.faces();
    for(auto f:f_circulator)
      {
	pfile << id_property[f] << " ";
	auto f_verts = mesh.vertices(f);
	for(auto v:f_verts)
	  pfile << v.idx()+1 << " " ;
	pfile << std::endl;
      }

    pfile.close();

    // done
    return;
  }


}
