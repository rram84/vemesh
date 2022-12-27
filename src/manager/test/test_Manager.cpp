// Sriramajayam

#include <vm_Manager.h>
#include <vm_io.h>
#include <vm_vertex_ring.h>
#include <fstream>
#include <set>

int main()
{
  //vm::Manager manager("coordinates.dat", "connectivity.dat");
  vm::Manager manager("slice.OFF");
  manager.write("mesh.off");
  manager.write_bad_angles("bad_angles.off", 20.);

  // merge
  int nmerged = manager.merge_faces(20.);
  manager.inspect_mesh();
  std::cout << "Merged "<<nmerged << " elements "<< std::endl;
  manager.write("merged.off");
  pmp::SurfaceMesh merged_mesh = manager.get_mesh();
  vm::write_dat(manager.get_mesh(), "merged.dat");
  
  // move vertices
  std::set<pmp::Vertex> moved_verts{};
  auto& mesh = manager.get_mesh();
  auto v_circulator = mesh.vertices();
  for(auto vertex:v_circulator)
    if(!mesh.is_boundary(vertex))
      {
	auto lc_1 = vm::compute_distance_based_vertex_quality(mesh, vertex);
	auto havg = vm::compute_average_edge_length_at_vertex(mesh, vertex);
	if(lc_1.radius/havg < 0.2)
	  {
	    auto move_result = manager.move_vertex(vertex, 20);
	    if(move_result.first==true)
	      {
		std::cout << "Moving vertex " << vertex.idx() << std::endl;
		moved_verts.insert(vertex);
		auto lc_2 = move_result.second;
		assert(lc_1.radius<=lc_2.radius);
	      }
	  }
      }
  manager.inspect_mesh();
  manager.write("moved.off");
  vm::write_dat(mesh, "moved.dat");

  // vertex qualities before movement
  std::fstream pfile;
  pfile.open("pre-circles.dat", std::ios::out);
  assert(pfile.good());
  for(auto& v:moved_verts)
    {
      auto lc = vm::compute_distance_based_vertex_quality(merged_mesh, v);
      pfile << lc.center[0] << " " << lc.center[1] << " " << lc.radius << std::endl;
    }
  pfile.close();
      
  // qualities after movement
  pfile.open("post-circles.dat", std::ios::out);
  assert(pfile.good());
  for(auto& v:moved_verts)
    {
      auto lc = vm::compute_distance_based_vertex_quality(mesh, v);
      pfile << lc.center[0] << " " << lc.center[1] << " " << lc.radius << std::endl;
    }
  pfile.close();
  
  // snap
  /*manager.write_bad_vertices("bad_edges.off", 0.1);
    int nsnaps = manager.snap_vertices(0.1);
    manager.inspect_mesh();
    std::cout << "Snapped " << nsnaps << " vertices " << std::endl;
    manager.write("snapped-1.off");
  
    nsnaps = manager.snap(0.2);
    manager.inspect_mesh();
    std::cout << "Snapped " << nsnaps << " vertices " << std::endl;
    manager.write("snapped-2.off");

    nsnaps = manager.snap(0.25);
    manager.inspect_mesh();
    std::cout << "Snapped " << nsnaps << " vertices " << std::endl;
    manager.write("snapped-3.off");*/  
}
