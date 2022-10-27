// Sriramajayam

#include <vm_Manager.h>
#include <vm_io.h>
#include <vm_quality.h>

namespace vm
{
  // Constructor
  Manager::Manager(const std::string coord_file, const std::string conn_file)
  {
    mesh.clear();
    curr2ref_vertex_map.clear();
    read_triangles(coord_file, conn_file, mesh);

    // only triangles
    assert(mesh.is_triangle_mesh()==true);
    
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
  void Manager::write(const std::string filename) 
  {
    if(curr2ref_vertex_map.empty())
      write_off(mesh, filename);
    else
      write_off(mesh, curr2ref_vertex_map, filename);
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
    std::list<pmp::Face> facelist;
    facelist.clear();
    auto vert_circulator = mesh.vertices();
    for(auto vert:vert_circulator)
      {
	auto halfedge_circulator = mesh.halfedges(vert);
	std::vector<double> edge_len{};
	const auto& Xa = mesh.position(vert);
	for(auto h:halfedge_circulator)
	  {
	    const auto& Xb = mesh.position(mesh.to_vertex(h));
	    edge_len.push_back(std::sqrt((Xa[0]-Xb[0])*(Xa[0]-Xb[0]) + (Xa[1]-Xb[1])*(Xa[1]-Xb[1])));
	  }
	const auto minmax = std::minmax_element(edge_len.begin(), edge_len.end());

	if((*minmax.first)/(*minmax.second)<eps_edge_ratio) // small edge at this vertex
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
