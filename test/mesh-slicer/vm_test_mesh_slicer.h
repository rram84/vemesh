// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <functional>
#include <map>

namespace vm
{
  namespace test
  {
    using LevelSetFunction_t = std::function<double(const double*)>;

    int adjust_mesh_nodes(pmp::SurfaceMesh& mesh, const double phi_eps, const double pert_eps, LevelSetFunction_t& ls_func);
  
    void clip_mesh(pmp::SurfaceMesh& mesh, const double phi_eps, LevelSetFunction_t& lsfunc);

    void embed_interface(pmp::SurfaceMesh& mesh, const double phi_eps, LevelSetFunction_t& lsfunc, const std::pair<int,int> domain_id);

    void prep_mesh(pmp::SurfaceMesh& mesh, const double phi_eps, LevelSetFunction_t& lsfunc,
		   const bool discard_outer_faces,
		   const std::pair<int,int> domain_id,                        // id for inner domain, outer domain
		   std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,             // cut edges -> new vertex map
		   std::map<pmp::Face, std::vector<int>>& cutfacesMap);       // cut faces -> local vertices in phi<0
		   

    void slice_triangle(pmp::SurfaceMesh& mesh,
			const std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
			const pmp::Face& e, const std::vector<int>& in_verts,
			const bool discard_outer,             // discard the element portion in phi>0
			const std::pair<int,int> domain_id);  // {inner domain id, outer domain id}

    void slice_quad(pmp::SurfaceMesh& mesh,
		    const std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
		    const pmp::Face& e,  std::vector<int> in_verts,
		    const bool discard_outer,               // discard the element portion in phi>0
		    const std::pair<int,int> domain_id);    // {inner domain id, outer domain id}
    
  }
}
