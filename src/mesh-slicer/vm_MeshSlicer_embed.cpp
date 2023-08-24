// Sriramajayam

#include <vm_MeshSlicer.h>

namespace vm
{
  void embed_interface(pmp::SurfaceMesh& mesh, const double phi_eps, LevelSetFunction_t& lsfunc,
		       const std::pair<int,int> domain_id)
  {
    // cut edges -> new vertex map
    std::map<pmp::Edge, pmp::Vertex> cutedgesMap{};
    
    // cut faces -> local vertices in phi<0
    std::map<pmp::Face, std::vector<int>> cutfacesMap{};

    // don't dicard region phi>0 of the mesh
    const bool discard_outer = false;
    slicer::prep_mesh(mesh, phi_eps, lsfunc, discard_outer, // in
		      cutedgesMap, cutfacesMap);            // out

    // emed triangles and quads
    for(auto& it:cutfacesMap)
      {
	// face to clip and its local inner vertices
	const auto& e        = it.first;
	const auto& in_verts = it.second;
	const int n_verts    = mesh.valence(e);

	// embed a triangle/quad
	if(n_verts==3)
	  slicer::slice_triangle(mesh, cutedgesMap, e, in_verts, discard_outer);
	else
	  slicer::slice_quad(mesh, cutedgesMap, e, in_verts, discard_outer);
      }
    
    // done
    return;
  }
}
