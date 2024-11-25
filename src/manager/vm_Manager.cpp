// Sriramajayam

#include <vm_Manager.h>
#include <vm_io.h>
#include <filesystem>

namespace vm
{
  // Constructor: from a .vtk or .OFF file
  Manager::Manager(const std::string filename)
  {
    mesh.clear();
    const std::string ext = std::filesystem::path(filename).extension();
    assert(ext==".vtk" || ext==".off" || ext==".OFF");
    if(ext==".vtk")
      read_vtk(filename, mesh);
    else
      read_off(filename, mesh);

    // sanity checks
    inspect_mesh();
  }
  
  // visualize
  void Manager::write_mesh(const std::string filename) 
  {
    // distinguish file formats
    const std::string extension = std::filesystem::path(filename).extension();
    if(extension==".off" || extension==".OFF")
      write_off(mesh, filename);
    else if(extension==".dat")
      write_dat(mesh, filename);
    else if(extension==".vtk")
      write_vtk(mesh, filename);
    
    return;
  }

  
  // visualize mesh along with face qualities
  void Manager::compute_face_qualities(FaceQuality_f qfunc)
  {
    // "face_quality" property
    if(mesh.has_face_property("face_quality")==false)
      {
	mesh.add_face_property<double>("face_quality");
      }

    // (re)compute
    auto quality = mesh.get_face_property<double>("face_quality");
    auto f_circulator = mesh.faces();
    double q;
    for(auto f:f_circulator)
      {
	q = MeshFaceQuality_f(mesh, f, qfunc);
	quality[f] = q;
      }

    // done
    return;
  }
  
  
  // visualize mesh along with vertex qualities
  void Manager::compute_vertex_qualities(MeshVertexQuality_f qfunc)
  {
    // "vertex_quality" property
    if(mesh.has_vertex_property("vertex_quality")==false)
      {
	mesh.add_vertex_property<double>("vertex_quality");
      }

    // (re)compute
    auto quality = mesh.get_vertex_property<double>("vertex_quality");
    auto v_circulator = mesh.vertices();
    double q;
    for(auto v:v_circulator)
      {
	q = qfunc(mesh, v);
	quality[v] = q;
      }

    // done
    return;
  }

}
