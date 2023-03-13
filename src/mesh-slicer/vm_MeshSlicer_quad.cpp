// Sriramajayam

#include <vm_MeshSlicer.h>
#include <cassert>
#include <list>
#include <map>


namespace vm
{
  void clip_quad_mesh(pmp::SurfaceMesh& mesh, LevelSetFunction_t& lsfunc)
  {
    // cut edges -> new vertex map
    std::map<pmp::Edge, pmp::Vertex> cutedgesMap{};

    // cut faces -> local inner vertices map
    std::map<pmp::Face, std::vector<int>> cutfacesMap{};

    // remove outer faces
    // insert vertices along cut edges
    // identify cut faces and their local inner vertices
    clip_mesh_prep(mesh, 4, lsfunc, cutedgesMap, cutfacesMap);

    // clip quads
    for(auto& it:cutfacesMap)
      {
	// quad to clip and its local inner vertices
	const auto& e        = it.first;
	const auto& in_verts = it.second;
	const int n_in_verts = static_cast<int>(in_verts.size());
	assert(n_in_verts>0 && n_in_verts<4);

	// outer vertices
	const std::vector<int> all_verts{0,1,2,3};
	std::vector<int> out_verts{};
	std::set_difference(all_verts.begin(), all_verts.end(), in_verts.begin(), in_verts.end(),
			    std::back_inserter(out_verts));
	
	// vertices of this face
	auto v_circulator = mesh.vertices(e);
	std::vector<pmp::Vertex> my_verts{};
	for(auto v:v_circulator)
	  my_verts.push_back(v);

	// 1-in, 3-out
	if(n_in_verts==1)
	  {
	    // cut edges emanating from the inner vertex
	    const int a0 = in_verts[0];
	    const int a1 = (a0+1)%4;
	    const int a2 = (a0+3)%4;
	    const auto e0 = mesh.find_edge(my_verts[a0], my_verts[a1]);
	    const auto e1 = mesh.find_edge(my_verts[a2], my_verts[a0]);

	    // vertices inserted along e0 and e1
	    auto it = cutedgesMap.find(e0);
	    auto jt = cutedgesMap.find(e1);
	    assert(it!=cutedgesMap.end() && jt!=cutedgesMap.end());

	    // erase the old face
	    mesh.delete_face(e);

	    // create new face
	    mesh.add_face({my_verts[a0], it->second, jt->second});
	  }
	// 3-in, 1-out
	else if(n_in_verts==3) 
	  {
	    const int a0 = out_verts[0];
	    const int a1 = (a0+1)%4;
	    const int a2 = (a0+2)%4;
	    const int a3 = (a0+3)%4;
	    const auto e0 = mesh.find_edge(my_verts[a0], my_verts[a1]);
	    const auto e1 = mesh.find_edge(my_verts[a3], my_verts[a0]);

	    // vertices inserted along e0 and e1
	    auto it = cutedgesMap.find(e0);
	    auto jt = cutedgesMap.find(e1);
	    assert(it!=cutedgesMap.end() && jt!=cutedgesMap.end());

	    // erase the old face
	    mesh.delete_face(e);

	    // create new face
	    mesh.add_face({it->second, my_verts[a1], my_verts[a2], my_verts[a3], jt->second});
	  }
	// 2-in, 2-out
	else if(n_in_verts==2)
	  {
	    // inner vertices should be successive
	    assert(in_verts[1]==(in_verts[0]+1)%4);
	    const int a0 = in_verts[0];
	    const int a1 = in_verts[1];  // (a0+1)%4
	    const int a2 = (a0+2)%4;
	    const int a3 = (a0+3)%4;
	    const auto e0 = mesh.find_edge(my_verts[a1], my_verts[a2]);
	    const auto e1 = mesh.find_edge(my_verts[a3], my_verts[a0]);

	    // vertices inserted along e0 and e1
	    auto it = cutedgesMap.find(e0);
	    auto jt = cutedgesMap.find(e1);
	    assert(it!=cutedgesMap.end() && jt!=cutedgesMap.end());

	    // erase the old face
	    mesh.delete_face(e);

	    // add new face
	    mesh.add_face({it->second, my_verts[a2], my_verts[a3], jt->second});
	  }
      }

    // renumber the mesh
    mesh = renumber_mesh_vertices(mesh);

    // done
    return;
  }
}
