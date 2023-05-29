// Sriramajayam

#include <vm_Manager.h>
#include <vm_io.h>
#include <filesystem>

namespace vm
{
  // Constructor: from a .OFF file
  Manager::Manager(const std::string off_file)
  {
    mesh.clear();
    read_off(off_file, mesh);

    // sanity checks
    inspect_mesh();
  }
  

  // Constructor: from .node, .ele files
  Manager::Manager(const std::string node_file, const std::string ele_file)
  {
    mesh.clear();
    read_triangles(node_file, ele_file, mesh);

    // sanity checks
    inspect_mesh();
  }
  
  
  // Destructor
  Manager::~Manager() {}


  // access the mesh
  pmp::SurfaceMesh& Manager::get_mesh()
  {
    return mesh;
  }

  // visualize
  void Manager::write_mesh(const std::string filename) 
  {
    // distinguish file formats
    const std::string extension = std::filesystem::path(filename).extension();
    if(extension==".off" || extension==".OFF")„
      write_off(mesh, filename);
    else if(extension==".dat")
      write_dat(mesh, filename);
    else if(extension==".vtk")
      write_vtk(mesh, filename);
    
    return;
  }

  
  // visualize mesh along with face qualities
  void Manager::write_mesh(const std::string filename, MeshFaceQuality_f qfunc)
  {
    // Only vtk is supported
    const std::string extension = std::filesystem::path(filename).extension();
    assert(extension==".vtk" && "Only vtk file format is supported");
    write_vtk_with_cell_data(mesh, qfunc, filename);

    return;
  }

  
  // visualize mesh along with vertex qualities
  void Manager::write_mesh(const std::string filename, MeshVertexQuality_f qfunc)
  {
    // Only vtk is supported
    const std::string extension = std::filesystem::path(filename).extension();
    assert(extension==".vtk" && "Only vtk file format is supported");
    write_vtk_with_vertex_data(mesh, qfunc, filename);

    return;
  }


  
}
