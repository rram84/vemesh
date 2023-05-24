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
    write_off(mesh, filename);
    return;
  }
  
  // visualize elements with poor qualities
  void Manager::write_bad_faces(const std::string filename, const double qeps, MeshFaceQuality_f qfunc)
  {
    std::list<pmp::Face> facelist;
    facelist.clear();
    auto face_circulator = mesh.faces();
    for(auto face:face_circulator)
      {
	double quality = qfunc(mesh, face);
	if(quality<qeps)
	  facelist.push_back(face);
      }
    write_off(mesh, facelist, filename);
    return;
  }
  
  // visualize elements with poor quality
  void Manager::write_bad_vertices(const std::string filename, const double qeps, MeshVertexQuality_f qfunc)
  {
    std::list<pmp::Face> facelist;
    facelist.clear();
    auto vert_circulator = mesh.vertices();
    for(auto vert:vert_circulator)
      {
	double quality = qfunc(mesh, vert);
	if(quality<qeps)
	  {
	    auto face_circulator = mesh.faces(vert);
	    for(auto f:face_circulator)
	      facelist.push_back(f);
	  }
      }
    
    write_off(mesh, facelist, filename);
    return;	
  }

}
