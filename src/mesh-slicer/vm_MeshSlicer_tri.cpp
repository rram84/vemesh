// Sriramajayam

#include <vm_MeshSlicer.h>
#include <cassert>
#include <list>
#include <map>

namespace vm
{
  void clip_tri_mesh(pmp::SurfaceMesh& mesh, LevelSetFunction_t& lsfunc)
  {
    // cut edges -> new vertex map
    std::map<pmp::Edge, pmp::Vertex> cutedgesMap{};
    
    // cut faces -> local inner vertices map
    std::map<pmp::Face, std::vector<int>> cutfacesMap{};

    // remove outer faces
    // insert vertices along cut edges
    // identify cut faces, and their local inner vertices
    clip_mesh_prep(mesh, 3, lsfunc, cutedgesMap, cutfacesMap);
    
    // clip triangles
    for(auto& it:cutfacesMap)
      {
	// triangle to clip and its local inner vertices
	const auto& e        = it.first;
	const auto& in_verts = it.second;
	const int n_in_verts = static_cast<int>(in_verts.size());
	assert(n_in_verts==1 || n_in_verts==2);

	// vertices of this face
	auto v_circulator = mesh.vertices(e);
	std::vector<pmp::Vertex> my_verts{};
	for(auto v:v_circulator)
	  my_verts.push_back(v);
	
	// 1-in, 2-out case
	if(n_in_verts==1)
	  {
	    // edges emanating from the inner vertex
	    const int a0 = in_verts[0];
	    const int a1 = (a0+1)%3;
	    const int a2 = (a1+1)%3;
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
	else // 2-in, 1-out case
	  {
	    int out_vert = -1;
	    if(in_verts[0]==0 && in_verts[1]==1)
	      out_vert = 2;
	    else if(in_verts[0]==0 && in_verts[1]==2)
	      out_vert = 1;
	    else if(in_verts[0]==1 && in_verts[1]==2)
	      out_vert = 0;
	    else
	      assert(false);
	    
	    // edges emanating from the outer vertex
	    const int a0 = out_vert;
	    const int a1 = (a0+1)%3;
	    const int a2 = (a1+1)%3;
	    const auto e0 = mesh.find_edge(my_verts[a0], my_verts[a1]);
	    const auto e1 = mesh.find_edge(my_verts[a2], my_verts[a0]);

	    // vertices inserted along e0 and e1
	    auto it = cutedgesMap.find(e0);
	    auto jt = cutedgesMap.find(e1);
	    assert(it!=cutedgesMap.end() && jt!=cutedgesMap.end());

	    // erase the old face
	    mesh.delete_face(e);

	    // create new face
	    mesh.add_face({it->second, my_verts[a1], my_verts[a2], jt->second});
	  }
      }
    
    // renumber the mesh
    mesh = renumber_mesh_vertices(mesh);
     
    // done
    return;
  }


}
