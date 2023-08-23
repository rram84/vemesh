// Sriramajayam

#include <vm_MeshSlicer.h>
#include <cassert>
#include <map>

namespace vm
{
  pmp::SurfaceMesh renumber_mesh_vertices(const pmp::SurfaceMesh& mesh);
  
  void clip_mesh(pmp::SurfaceMesh& mesh, const double phi_eps, LevelSetFunction_t& lsfunc)
  {
    // cut edges -> new vertex map
    std::map<pmp::Edge, pmp::Vertex> cutedgesMap{};

    // cut faces -> local vertices in phi<0
    std::map<pmp::Face, std::vector<int>> cutfacesMap{};

    // discard faces that lie within the phi>0 domain
    const bool discard_outer = true;
    slicer::prep_mesh(mesh, phi_eps, lsfunc, discard_outer,  // in
		      cutedgesMap, cutfacesMap);             // out
    
    // clip triangles and quads
    for(auto& it:cutfacesMap)
      {
	// face to clip and its local inner vertices
	const auto& e        = it.first;
	const auto& in_verts = it.second;
	const int n_verts    = mesh.valence(e);
	assert(n_verts==3 || n_verts==4);
	
	// clip a triangle/quad
	if(n_verts==3)
	  slicer::slice_triangle(mesh, cutedgesMap, e, in_verts, discard_outer);
	else 
	  slicer::slice_quad(mesh, cutedgesMap, e, in_verts, discard_outer);
      }
    
    // renumber the mesh
    mesh = renumber_mesh_vertices(mesh);
     
    // done
    return;
  }

  
  
  pmp::SurfaceMesh renumber_mesh_vertices(const pmp::SurfaceMesh& mesh)
  {
    pmp::SurfaceMesh renum_mesh;
    
    // add renumbered vertices to the new mesh
    std::map<pmp::Vertex,int> old2new{};
    int vcount = 0;
    auto v_container = mesh.vertices();
    std::vector<pmp::Vertex> new_vertices{};
    for(auto v:v_container)
      {
	old2new.insert({v, vcount++});
	new_vertices.push_back( renum_mesh.add_vertex(mesh.position(v)) );
      }

    // add renumbered faces to the new mesh
    auto f_container = mesh.faces();
    for(auto f:f_container)
      {
	auto v_circulator = mesh.vertices(f);
	std::vector<pmp::Vertex> renum_face_verts{};
	for(auto v:v_circulator)
	  {
	    auto it = old2new.find(v);
	    assert(it!=old2new.end());
	    renum_face_verts.push_back(new_vertices[it->second]);
	  }
	renum_mesh.add_face(renum_face_verts);
      }

    // sanity checks
    assert(mesh.n_vertices()==renum_mesh.n_vertices());
    assert(mesh.n_edges()==renum_mesh.n_edges());
    assert(mesh.n_faces()==renum_mesh.n_faces());
    
    // done
    return renum_mesh;
  }

		      
}
