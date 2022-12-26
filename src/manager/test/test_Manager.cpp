// Sriramajayam

#include <vm_Manager.h>
#include <vm_io.h>
#include <vm_vertex_ring.h>
#include <fstream>

int main()
{
  vm::Manager manager("coordinates.dat", "connectivity.dat");
  //vm::Manager manager("slice.OFF");
  manager.write("mesh.off");
  manager.write_bad_angles("bad_angles.off", 20.);

  // merge
  int nmerged = manager.merge_faces(20.);
  manager.inspect_mesh();
  std::cout << "Merged "<<nmerged << " elements "<< std::endl;
  manager.write("merged.off");
  //vm::write_dat(manager.get_mesh(), "merged.dat");
  
  // move vertices
  int num_verts_moved   = 0;
  int num_verts_unmoved = 0;
  auto& mesh = manager.get_mesh();
  auto v_circulator = mesh.vertices();
  std::map<pmp::Vertex, vm::LimitCircle_t> pre_limits{};
  for(auto vertex:v_circulator)
    if(!mesh.is_boundary(vertex))
      {
	auto lc_1 = vm::compute_distance_based_vertex_quality(mesh, vertex);
	auto havg = vm::compute_average_edge_length_at_vertex(mesh, vertex);
	if(lc_1.radius/havg < 0.2)
	  {
	    std::cout << vertex.idx() << std::endl;
	    auto move_result = manager.move_vertex(vertex, 20);
	    if(move_result.first==true)
	      {
		pre_limits.insert({vertex, lc_1});
		++num_verts_moved;
		auto lc_2 = move_result.second;
		assert(lc_1.radius<=lc_2.radius);
	      }
	    else
	      ++num_verts_unmoved;
	  }
      }
  manager.inspect_mesh();
  std::cout << "Moved " << num_verts_moved << " vertices, unsuccessful at " << num_verts_unmoved << std::endl;
  manager.write("moved.off");
  vm::write_dat(manager.get_mesh(), "moved.dat");
  
  // qualities after movement
  std::map<pmp::Vertex, vm::LimitCircle_t> post_limits{};
  for(auto& it:pre_limits)
    post_limits.insert({it.first, vm::compute_distance_based_vertex_quality(mesh, it.first)});

  std::cout << "Vertex quality comparison: " << std::endl;
  for(auto& it:pre_limits)
    {
      auto jt = post_limits.find(it.first);
      assert(jt!=post_limits.end());
      std::cout << it.first.idx() <<" : " << it.second.radius << " --> " << jt->second.radius << std::endl;
    }

	// limiting circles before movement
	std::fstream pfile;
      pfile.open("pre-circles.dat", std::ios::out);
      assert(pfile.good());
      for(auto& it:pre_limits)
	pfile << it.second.center[0] << " " << it.second.center[1] << " " << it.second.radius << std::endl;
      pfile.close();

      // limiting circles after movement
      pfile.open("post-circles.dat", std::ios::out);
      assert(pfile.good());
      for(auto& it:post_limits)
	pfile << it.second.center[0] << " " << it.second.center[1] << " " << it.second.radius << std::endl;
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
