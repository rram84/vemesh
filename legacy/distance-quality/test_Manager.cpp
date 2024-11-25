// Sriramajayam

#include <vm_Manager.h>
#include <vm_io.h>
#include <vm_vertex_ring.h>
#include <fstream>
#include <set>

int main()
{
  vm::Manager manager("slice.OFF");
  auto& mesh = manager.get_mesh();
  
  // algorithm: alternately merge triangles and move vertices
  const int nIters = 10;
  for(int iter=0; iter<nIters; ++iter)
    {
      std::cout << std::endl << "Iteration " << iter << std::endl;

      // merge triangles
      int num_trias_merged = manager.agglomerate_triangles(20.);
      std::cout << "Merged " << num_trias_merged << " triangles " << std::endl;
      manager.write("merged-"+std::to_string(iter)+".off");
      
      // move nodes
      auto v_circulator = mesh.vertices();
      int move_count = 0;
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
		    ++move_count;
		    auto lc_2 = move_result.second;
		    assert(lc_1.radius<=lc_2.radius);
		  }
	      }
	  }
      manager.inspect_mesh();

      std::cout << "Moved " << move_count << " vertices " << std::endl;
      manager.write("moved-" + std::to_string(iter)+".off");
    }

  /*
  // vertex qualities before movement
  std::vector<double> pre_qualities{};
  std::fstream pfile;
  pfile.open("pre-circles.dat", std::ios::out);
  assert(pfile.good());
  for(auto& v:moved_verts)
    {
      auto lc = vm::compute_distance_based_vertex_quality(merged_mesh, v);
      pfile << lc.center[0] << " " << lc.center[1] << " " << lc.radius << std::endl;
      pre_qualities.push_back(lc.radius);
    }
  pfile.close();
      
  // qualities after movement
  std::vector<double> post_qualities{};
  pfile.open("post-circles.dat", std::ios::out);
  assert(pfile.good());
  for(auto& v:moved_verts)
    {
      auto lc = vm::compute_distance_based_vertex_quality(mesh, v);
      pfile << lc.center[0] << " " << lc.center[1] << " " << lc.radius << std::endl;
      post_qualities.push_back(lc.radius);
    }
  pfile.close();

  std::sort(pre_qualities.begin(),  pre_qualities.end());
  std::sort(post_qualities.begin(), post_qualities.end());
  pfile.open("qualities.dat", std::ios::out);
  const int nEntries = static_cast<int>(pre_qualities.size());
  for(int i=0; i<nEntries; ++i)
    pfile << i+1 << " " << pre_qualities[i] << " " << post_qualities[i] << std::endl;
  pfile.close();
  */
}
