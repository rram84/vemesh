// Sriramajayam

#include <vm_test_mesh_slicer.h>

namespace vm
{
  namespace test
  {
    void embed_interface(pmp::SurfaceMesh& mesh, const double phi_eps, LevelSetFunction_t& lsfunc,
			 const std::pair<int,int> domain_id)
    {
      // domain id
      if(mesh.has_face_property("id")==false)
	mesh.add_face_property<int>("id", -1);
    
      // cut edges -> new vertex map
      std::map<pmp::Edge, pmp::Vertex> cutedgesMap{};
    
      // cut faces -> local vertices in phi<0
      std::map<pmp::Face, std::vector<int>> cutfacesMap{};

      // don't dicard region phi>0 of the mesh
      const bool discard_outer = false;
      prep_mesh(mesh, phi_eps, lsfunc,
		discard_outer, domain_id, 
		cutedgesMap, cutfacesMap);            

      // emed triangles and quads
      for(auto& it:cutfacesMap)
	{
	  // face to clip and its local inner vertices
	  const auto& e        = it.first;
	  const auto& in_verts = it.second;
	  const int n_verts    = mesh.valence(e);

	  // embed a triangle/quad
	  if(n_verts==3)
	    slice_triangle(mesh, cutedgesMap, e, in_verts, discard_outer, domain_id);
	  else
	    slice_quad(mesh, cutedgesMap, e, in_verts, discard_outer, domain_id);
	}
    
      // done
      return;
    }
  }
}
