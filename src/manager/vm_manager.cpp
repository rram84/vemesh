// Sriramajayam

#include <vm_manager.h>
#include <vm_io.h>
#include <vm_inspect.h>
#include <vm_quality.h>

namespace vm
{
  // Constructor
  Manager::Manager(const std::string coord_file, const std::string conn_file)
  {
    mesh.clear();
    read_triangles(coord_file, conn_file, mesh);

    // only triangles
    auto face_circulator = mesh.faces();
    for(auto face:face_circulator)
      assert(mesh.valence(face)==3);
    
    // sanity checks
    inspect_mesh();
  }

  // Destructor
  Manager::~Manager() {}

  // inspect validity of the mesh
  void Manager::inspect_mesh()
  {
    assert(mesh.n_vertices()>0);
    assert(mesh.n_faces()>0);
    assert(mesh.n_edges()>0);

    // inspect faces
    auto face_circulator = mesh.faces();
    for(auto face:face_circulator)
      inspect_face(mesh, face);

    // done
    return;
  }

  
  // visualize
  void Manager::write(const std::string filename) 
  {
    write_off(mesh, filename);
    return;
  }
  
  // visualize elements with small angles
  void Manager::write_bad_angles(const std::string filename, const double eps_degrees) 
  {
    std::list<pmp::Face> facelist;
    facelist.clear();
    auto face_circulator = mesh.faces();
    for(auto face:face_circulator)
      {
	double min_angle = face_quality(mesh, face);
	if(min_angle<eps_degrees)
	  facelist.push_back(face);
      }
    write_off(mesh, facelist, filename);
    return;
  }
  
  // visualize elements with small edges
  void Manager::write_bad_vertices(const std::string filename, const double eps_edge_ratio) 
  {
  }

}
