// Sriramajayam

#include <vm_tutorial_mesh_slicer.h>
#include <random>
#include <iostream>
#include <map>

namespace vm {
  namespace tutorial {

    // adjust mesh nodes away from the zero level set
    void adjust_mesh_nodes(pmp::SurfaceMesh& mesh,
			   const double phi_eps,
			   const double pert_eps,
			   const LevelSetFunction_t& ls_func)
    {
      if (phi_eps <= 0.0)
        throw std::invalid_argument("adjust_mesh_nodes: phi_eps must be positive");
      
      if (pert_eps <= 0.0)
        throw std::invalid_argument("adjust_mesh_nodes: pert_eps must be positive");
      
      double sd;
      int nadjusted = 0;

      std::mt19937 gen(std::random_device{}());
      std::uniform_real_distribution<double> distr(-1.,1.);

      auto v_container = mesh.vertices();
      for(auto v:v_container)
	{
	  pmp::Point& X = mesh.position(v);
	  double Y[3];
	  for(int k=0; k<3; ++k)
	    Y[k] = X[k];
	  
	  sd = ls_func(Y);
	  if(std::abs(sd)<phi_eps)
	    {
	      // adjust the node position until |phi| > phi_eps
	      int iter = 0;
	      while(true)
		{
		  // permit a max of 10 iterations
		  if (iter >= 10)
		    {
		      throw std::runtime_error(
					       "adjust_mesh_nodes: failed to move vertex away from interface "
					       "after 10 perturbations"
					       );
		    }
		  
		  // generate a random perturbation direction
		  double dir[] = {distr(gen), distr(gen)};
		  double norm = std::sqrt(dir[0]*dir[0]+dir[1]*dir[1]);
		  if(norm<1.e-6)
		    continue;

		  // perturbation vector
		  const double dx = pert_eps*dir[0]/norm;
		  const double dy = pert_eps*dir[1]/norm;

		  // try perturbing
		  Y[0] += dx;
		  Y[1] += dy;
		  // Z coordinate is a dummy

		  sd = ls_func(Y);
		  if (std::abs(sd) > phi_eps)
		    break;
		  else
		    {
		      // undo the perturbation
		      Y[0] -= dx;
		      Y[1] -= dy;
		    }

		  // next iteration
		  ++iter;
		}
	      
	      // update node position
	      for(int k=0; k<3; ++k)
		X[k] = Y[k];

	      // update the number of adjusted vertices.
	      ++nadjusted;
	    }
	}
      std::cout << "adjust_mesh_nodes: Successfully adjusted " << nadjusted
		<< " vertices away from the zero level set.\n";
      return;
    }
  }
}

// Helper functions
namespace {

  // Classify faces with respect to the level-set function and prepare data
  // structures for subsequent slicing.
  //
  //  Assumes: Level-set values at existing vertices satisfy |phi| > phi_eps (a tolerance)
  //
  // Should be called after adjust_mesh_nodes()
  // 
  // - Evaluates the level-set function at all vertices
  // - Identifies faces intersected by the zero level set
  // - Optionally deletes faces lying entirely in phi > 0
  // - Inserts new vertices along cut edges
  // - Populates:
  //     * cutedgesMap  : map from cut edge -> newly inserted vertex
  //     * cutfacesMap  : map from cut face -> local indices of vertices with phi < 0
  //
  // Assumes:
  // - Level-set values at existing vertices satisfy |phi| > phi_eps
  // - mesh has face property "domain_id"
  // - mesh has vertex property "interface_id"
  void prep_mesh(pmp::SurfaceMesh& mesh,
		 const double phi_eps,
		 const vm::tutorial::LevelSetFunction_t& lsfunc,
		 const bool discard_outer_faces,
		 const std::pair<int,  int> domain_id,     // id for inner domain, outer domain
		 const int interface_id,                                    // interface id
		 std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,             // cut edges -> new vertex map
		 std::map<pmp::Face, std::vector<int>>& cutfacesMap);       // cut faces -> local vertices in phi<0
  

  // Slice a triangular face intersected by the zero level set.
  //
  // Assumes: Level-set values at existing vertices satisfy |phi| > phi_eps (a tolerance)
  // 
  // Assumes face vertex ordering follows the pmp::SurfaceMesh convention (counterclockwise, consistent orientation).
  // No reorientation is performed here.
  //
  // Should be called after prep_mesh()
  // 
  // Handles configurations:
  //   - 1 vertex inside, 2 outside
  //   - 2 vertices inside, 1 outside
  //
  // The original face is deleted and replaced by one or two new faces,
  // depending on whether outer portions are retained.
  //
  // Notes:
  // - New vertices are assumed to have already been inserted on cut edges
  // - Face domain IDs are assigned according to domain_id
  /*
    Triangle slicing cases:

    Case 1: 1 vertex inside (phi < 0), 2 outside
    v0 (in)
    *
    / \
    /   \
    v1     v2
    (out)  (out)

    - New vertices inserted along edges: e(v0,v1), e(v0,v2)
    - Inner face (domain_id=1) formed by: {v0, new_v_e0, new_v_e1}
    - Outer face (domain_id=2) formed by: {new_v_e0, v1, v2, new_v_e1} if discard_outer=false
    - Interface vertices (interface_id=1): new vertices on cut edges

    Case 2: 2 vertices inside, 1 outside
    v0 (in)       v1 (in)
    *-----------*
    \         /
    \       /
    v2 (out)

    - New vertices inserted along edges: e(v0,v2), e(v1,v2)
    - Inner face (domain_id=1): {new_v_e0, v0, v1, new_v_e1}
    - Outer face (domain_id=2): {v2, new_v_e0, new_v_e1} if discard_outer=false
  */
  void slice_triangle(pmp::SurfaceMesh& mesh,
		      const std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
		      const pmp::Face& e,
		      const std::vector<int>& in_verts,
		      const bool discard_outer,                                // discard the element portion in phi>0
		      const std::pair<int,  int> domain_id);  // {inner domain id, outer domain id}


  
  // Slice a quadrilateral face intersected by the zero level set.
  //
  // Assumes: Level-set values at existing vertices satisfy |phi| > phi_eps (a tolerance)
  //
  // Assumes face vertex ordering follows the pmp::SurfaceMesh convention (counterclockwise, consistent orientation).
  // No reorientation is performed here.
  //
  // Should be called after prep_mesh()
  // Supported configurations:
  //   - 1-in / 3-out
  //   - 3-in / 1-out
  //   - 2-in / 2-out (successive)
  //   - 2-in / 2-out (non-successive)
  //
  // The face is deleted and replaced by a set of triangles and/or polygons
  // representing the clipped geometry.
  //
  // Assumes:
  // - New vertices on cut edges exist in cutedgesMap
  // - Vertex ordering follows the original face ordering
  /*
    Quadrilateral slicing cases (vertices v0, v1, v2, v3 counterclockwise)
    * Labels: in/out indicates whether the vertex lies inside or outside the level set
    * For each case, the new element(s) created are indicated as inner (phi<0) and outer (phi>0)

    Case 1: 1-in / 3-out
    v0(in)----v1(out)
    *         *
    |         |
    *         *
    |         |
    v3(out)---v2(in)
    - Inner element (phi<0): triangle formed by v0 + intersections on edges (v0-v1 and v0-v3)
    - Outer element (phi>0, if retained): quad formed by v1,v2,v3 + intersections

    Case 2: 3-in / 1-out
    v0(in)----v1(in)
    *         *
    |         |
    *         *
    |         |
    v3(out)---v2(in)

    - Inner element: quad formed by v0,v1,v2 + intersections on edges to v3
    - Outer element: triangle formed by v3 + intersections

    Case 3: 2-in / 2-out (successive)
    v0(in)----v1(in)
    *         *
    |         |
    *         *
    |         |
    v3(out)---v2(out)

    - Inner element: quad formed by v0,v1 + intersections on edges to v2,v3
    - Outer element: quad formed by v2,v3 + intersections

    Case 4: 2-in / 2-out (non-successive)
    v0(in)----v1(out)
    *         *
    |         |
    *         *
    |         |
    v3(out)-----v2(in)

    - Inner element: hexagon formed by v0,v2 + intersections
    - Outer elements: two triangles, each formed by one outer vertex + adjacent intersect
  */
  void slice_quad(pmp::SurfaceMesh& mesh,
		  const std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
		  const pmp::Face& e,
		  std::vector<int> in_verts,
		  const bool discard_outer,                               // discard the element portion in phi>0
		  const std::pair<int,  int> domain_id); // {inner domain id, outer domain id}

  // This routine rebuilds a SurfaceMesh from scratch using the current
  // vertex coordinates and face connectivity.
  //
  // Motivation:
  //  - After clipping/slicing, the mesh may contain deleted faces,
  //    orphaned vertices, and non-contiguous vertex indices.
  //  - PMP does not automatically compact or renumber entities.
  //  - Rebuilding produces a clean mesh with contiguous vertex indexing
  //    and no deleted elements.
  //
  // Assumptions:
  //  - The input mesh has valid connectivity (no dangling halfedges).
  //  - Face orientations and vertex orderings are already correct.
  //  - Only renumbering of vertices, and resulting renumbering of connectivities is required; nothing else is performed
  //
  // Notes:
  //  - All vertices are copied in the order returned by mesh.vertices().
  //  - All faces are recreated using the corresponding new vertex handles.
  //  - Face and vertex properties are reattached explicitly as needed.
  //  - This is intentionally simple and meant only for tutorial use.
  //  - All faces are assigned a domain_id of 1
  //  - All boundary nodes are assigned an interface_id of 1, rest are assigned 0.
  pmp::SurfaceMesh rebuild_mesh(const pmp::SurfaceMesh& mesh);
}


namespace vm
{
  namespace tutorial
  {
    pmp::SurfaceMesh clip_mesh(const pmp::SurfaceMesh& in_mesh,
			       const double phi_eps,
			       const LevelSetFunction_t& lsfunc)
    {
      pmp::SurfaceMesh mesh = in_mesh;
      const std::pair<int,  int> in_out_domain_id{1, 0}; // in=1, out=0
      const int boundary_id = 1;
      
      // cut edges -> new vertex map
      std::map<pmp::Edge, pmp::Vertex> cutedgesMap{};

      // cut faces -> local vertices in phi<0
      std::map<pmp::Face, std::vector<int>> cutfacesMap{};

      // discard faces that lie within the phi>0 domain
      const bool discard_outer = true;
      prep_mesh(mesh, phi_eps, lsfunc, discard_outer, in_out_domain_id, boundary_id, cutedgesMap, cutfacesMap);

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
	    slice_triangle(mesh, cutedgesMap, e, in_verts, discard_outer, in_out_domain_id);
	  else 
	    slice_quad(mesh, cutedgesMap, e, in_verts, discard_outer, in_out_domain_id);
	}

      // renumber the mesh
      mesh = rebuild_mesh(mesh);
     
      // done
      return mesh;
    }

    pmp::SurfaceMesh embed_interface(const pmp::SurfaceMesh& in_mesh,
				     const double phi_eps,
				     const LevelSetFunction_t& lsfunc)
    {
      pmp::SurfaceMesh mesh = in_mesh;
      const std::pair<int,  int> domain_id {1,2}; // in=1, out=2
      const int interface_id = 1; // rest = -1
      
      // cut edges -> new vertex map
      std::map<pmp::Edge, pmp::Vertex> cutedgesMap{};
    
      // cut faces -> local vertices in phi<0
      std::map<pmp::Face, std::vector<int>> cutfacesMap{};

      // Preserve both subdomains during embedding
      const bool discard_outer = false;
      prep_mesh(mesh, phi_eps, lsfunc,
		discard_outer,
		domain_id, interface_id,
		cutedgesMap, cutfacesMap);            

      // embed triangles and quads
      // cutfacesMap is a snapshot computed before mesh mutation
      for(auto& it:cutfacesMap)
	{
	  // face to clip and its local inner vertices
	  const auto& e        = it.first;
	  const auto& in_verts = it.second;
	  const int n_verts    = mesh.valence(e);

	  // embed a triangle/quad
	  if(n_verts==3)
	    slice_triangle(mesh, cutedgesMap, e, in_verts, discard_outer, domain_id);
	  else if(n_verts==4)
	    slice_quad(mesh, cutedgesMap, e, in_verts, discard_outer, domain_id);
	  else
	    throw std::runtime_error("embed_interface: only triangles and quads supported");
	  
	}
    
      // done
      return mesh;
    }
  }
}




namespace {
  void prep_mesh(pmp::SurfaceMesh& mesh,
		 const double phi_eps,
		 const vm::tutorial::LevelSetFunction_t& lsfunc,
		 const bool discard_outer_faces,
		 const std::pair<int, int> domain_id,
		 const int interface_num,
		 std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
		 std::map<pmp::Face, std::vector<int>>& cutfacesMap)
  {
    assert(phi_eps>0.);
      
    // cut edges -> new vertex map
    cutedgesMap.clear();
    
    // cut faces -> # of vertices in phi<0
    cutfacesMap.clear();

    // domain id
    assert(mesh.has_face_property("domain_id")==true);
    auto dom_id = mesh.get_face_property<int>("domain_id");

    // interface_id
    assert(mesh.has_vertex_property("interface_id")==true);
    auto interface_id = mesh.get_vertex_property<int>("interface_id");
      
    // compute the level set function at the nodes
    auto lsvalues    = mesh.add_vertex_property<double>("level set values");
    auto v_container = mesh.vertices();
    double Y[2];
    double lsval;
    for(auto v:v_container)
      {
	const auto& X = mesh.position(v);
	Y[0] = X[0];
	Y[1] = X[1];
	lsval = lsfunc(Y);
	assert(std::abs(lsval)>phi_eps && "Level set function value at node violates tolerance");
	lsvalues[v] = lsval;
      }

    // identify faces intersected by the zero level set
    // if specified, erase faces lying completely outside
    const int n0_faces = mesh.n_faces();
    auto f_container = mesh.faces();
    std::vector<pmp::Face> faces_to_delete{};
    for(auto f:f_container)
      {
	std::vector<int> in_verts{};
	auto v_circulator = mesh.vertices(f);
	int acount = 0;
	for(auto v:v_circulator)
	  {
	    if(lsvalues[v]<0.)
	      in_verts.push_back(acount);
	    ++acount;
	  }
	  
	// at least one vertex outside => cut face
	const int num_verts   = mesh.valence(f);
	const int n_in_verts  =  static_cast<int>(in_verts.size());
	const int n_out_verts = num_verts-n_in_verts;

	if(n_in_verts==num_verts)              // inside face
	  dom_id[f] = domain_id.first;
	if(n_in_verts>0 && n_out_verts>0) 	 // cut face
	  cutfacesMap.insert({f, in_verts});
	else if(n_in_verts==0)                 // outside face
	  {
	    if(discard_outer_faces==true)
	      faces_to_delete.push_back(f);
	    else
	      dom_id[f] = domain_id.second;
	  }
      }

    // delete faces
    for(auto f:faces_to_delete)
      mesh.delete_face(f);
    
    // should have at least some faces left
    if(discard_outer_faces==true)
      assert(mesh.n_faces()>0 && "Mesh does not intersect subzero level set");
    else
      assert(mesh.n_faces()==n0_faces);

    // compute locations of new vertices on a per-edge basis
    // insert vertices along cut edges
    cutedgesMap.clear();
    auto e_container = mesh.edges();
    int vcount = mesh.n_vertices()+1;
    for(auto e:e_container)
      {
	const auto v0 = mesh.vertex(e, 0);
	const auto v1 = mesh.vertex(e, 1);
	const double& phi0 = lsvalues[v0];
	const double& phi1 = lsvalues[v1];
	if(phi0*phi1<0.)
	  {
	    const auto& x0 = mesh.position(v0);
	    const auto& x1 = mesh.position(v1);
	    double y[3];
	    if(phi0<0.)
	      {
		double lambda = phi1/(phi1-phi0);
		for(int k=0; k<3; ++k)
		  y[k] = lambda*x0[k]+(1.-lambda)*x1[k];
	      }
	    else
	      {
		double lambda = phi0/(phi0-phi1);
		for(int k=0; k<3; ++k)
		  y[k] = lambda*x1[k]+(1.-lambda)*x0[k];
	      }

	    // new vertex to add along this edge
	    auto new_vert = mesh.add_vertex(pmp::Point(y[0],y[1],y[2]));
	    cutedgesMap.insert({e, new_vert});

	    // assign boundary/interface id to the vertex
	    interface_id[new_vert] = interface_num;
	      
	  }
      }

    mesh.remove_vertex_property(lsvalues);
    assert(mesh.has_vertex_property("level set values")==false);

    // done
    return;
  }


  void slice_triangle(pmp::SurfaceMesh& mesh,
		      const std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
		      const pmp::Face& e,
		      const std::vector<int>& in_verts,
		      const bool discard_outer,
		      const std::pair<int,  int> domain_id)
  {
    const int n_in_verts = static_cast<int>(in_verts.size());
    assert(n_in_verts==1 || n_in_verts==2);

    // domain id
    assert(mesh.has_face_property("domain_id")==true);
    auto dom_id = mesh.get_face_property<int>("domain_id");
      
    // outer vertices
    const std::vector<int> all_verts{0,1,2};
    std::vector<int> out_verts{};
    std::set_difference(all_verts.begin(), all_verts.end(), in_verts.begin(), in_verts.end(),
			std::back_inserter(out_verts));
	
    // vertices of this face
    auto v_circulator = mesh.vertices(e);
    std::vector<pmp::Vertex> my_verts{};
    for(auto v:v_circulator)
      my_verts.push_back(v);

    // keep track of the number of mesh vertices
    const int nverts = mesh.n_vertices();

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

	// at most two vertices can be deleted
	assert(nverts-mesh.n_vertices()<=2);
	  
	// create new inner face
	auto f_in = mesh.add_face({my_verts[a0], it->second, jt->second});
	dom_id[f_in] = domain_id.first;

	// create new outer face
	if(discard_outer==false)
	  {
	    // no vertex can be deleted 
	    assert(mesh.n_vertices()==nverts && "Cannot clip boundary triangle with interface");
	    auto f_out = mesh.add_face({it->second, my_verts[a1], my_verts[a2], jt->second});
	    dom_id[f_out] = domain_id.second;
	  }
      }
    else // 2-in, 1-out case
      {
	// edges emanating from the outer vertex
	const int a0 = out_verts[0];
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

	// at most one vertex can be deleted
	assert(nverts-mesh.n_vertices()<=1);
	  
	// create inner new face
	auto f_in = mesh.add_face({it->second, my_verts[a1], my_verts[a2], jt->second});
	dom_id[f_in] = domain_id.first;

	// create new outer face
	if(discard_outer==false)
	  {
	    // no vertex can be deleted 
	    assert(mesh.n_vertices()==nverts && "Cannot clip boundary triangle with interface");
	    auto f_out = mesh.add_face({my_verts[a0], it->second, jt->second});
	    dom_id[f_out] = domain_id.second;
	  }
      }

    // done
    return;
  }



  void slice_quad(pmp::SurfaceMesh& mesh,
		  const std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
		  const pmp::Face& e,  std::vector<int> in_verts,
		  const bool discard_outer,
		  const std::pair<int,  int> domain_id)
  {
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

    // keep track of the number of mesh vertices
    const int nverts = mesh.n_vertices();

    // domain id
    assert(mesh.has_face_property("domain_id")==true);
    auto dom_id = mesh.get_face_property<int>("domain_id");
      
    // 1-in, 3-out
    if(n_in_verts==1)
      {
	// cut edges emanating from the inner vertex
	const int a0 = in_verts[0];
	const int a1 = (a0+1)%4;
	const int a2 = (a0+2)%4;
	const int a3 = (a0+3)%4;
	const auto e0 = mesh.find_edge(my_verts[a0], my_verts[a1]);
	const auto e3 = mesh.find_edge(my_verts[a3], my_verts[a0]);
	
	// vertices inserted along e0 and e3
	auto it = cutedgesMap.find(e0);
	auto jt = cutedgesMap.find(e3);
	assert(it!=cutedgesMap.end() && jt!=cutedgesMap.end());
	
	// erase the old face
	mesh.delete_face(e);

	// at most 3 vertices should be deleted
	assert(nverts-mesh.n_vertices()<=3);
	
	// create new inner face
	auto f_in = mesh.add_face({my_verts[a0], it->second, jt->second});
	dom_id[f_in] = domain_id.first;

	// create new outer face
	if(discard_outer==false)
	  {
	    // no vertex should be deleted when erasing the face
	    assert(nverts==mesh.n_vertices() && "Cannot clip boundary quad with interface");
	    auto f_out = mesh.add_face({it->second, my_verts[a1], my_verts[a2], my_verts[a3], jt->second});
	    dom_id[f_out] = domain_id.second;
	  }
      }
    // 3-in, 1-out
    else if(n_in_verts==3) 
      {
	const int a0 = out_verts[0];
	const int a1 = (a0+1)%4;
	const int a2 = (a0+2)%4;
	const int a3 = (a0+3)%4;
	const auto e0 = mesh.find_edge(my_verts[a0], my_verts[a1]);
	const auto e3 = mesh.find_edge(my_verts[a3], my_verts[a0]);
	
	// vertices inserted along e0 and e3
	auto it = cutedgesMap.find(e0);
	auto jt = cutedgesMap.find(e3);
	assert(it!=cutedgesMap.end() && jt!=cutedgesMap.end());
	
	// erase the old face
	mesh.delete_face(e);

	// at most 1 vertex should have been deleted
	assert(nverts-mesh.n_vertices()<=1);
	
	// create new inner face
	auto f_in = mesh.add_face({it->second, my_verts[a1], my_verts[a2], my_verts[a3], jt->second});
	dom_id[f_in] = domain_id.first;

	// create new outer face
	if(discard_outer==false)
	  {
	    // no vertex should be deleted when erasing the face
	    assert(nverts-mesh.n_vertices()==0 && "Cannot clip boundary quad with interface");
	    auto f_out = mesh.add_face({it->second, jt->second, my_verts[a0]});
	    dom_id[f_out] = domain_id.second;
	  }
      }
    // 2-in, 2-out, successive
    else if(n_in_verts==2 && ((in_verts[0]+1)%4==in_verts[1] || (in_verts[1]+1)%4==in_verts[0]))
      {
	// inner vertices should be successive
	if(in_verts[0]==0 && in_verts[1]==3)
	  {
	    in_verts[0] = 3;
	    in_verts[1] = 0;
	  }
	
	assert(in_verts[1]==(in_verts[0]+1)%4);
	const int a0 = in_verts[0];
	const int a1 = in_verts[1];  // (a0+1)%4
	const int a2 = (a0+2)%4;
	const int a3 = (a0+3)%4;
	const auto e1 = mesh.find_edge(my_verts[a1], my_verts[a2]);
	const auto e3 = mesh.find_edge(my_verts[a3], my_verts[a0]);
	    
	// vertices inserted along e1 and e3
	auto it = cutedgesMap.find(e1);
	auto jt = cutedgesMap.find(e3);
	assert(it!=cutedgesMap.end() && jt!=cutedgesMap.end());

	// erase the old face
	mesh.delete_face(e);
	    
	// at most 2 vertices should have been deleted
	assert(nverts-mesh.n_vertices()<=2);
	    
	// add new inner face
	auto f_in = mesh.add_face({it->second, jt->second, my_verts[a0], my_verts[a1]});
	dom_id[f_in] = domain_id.first;

	// add new outer face
	if(discard_outer==false)
	  {
	    // no vertex should be deleted when erasing the face
	    assert(nverts-mesh.n_vertices()==0 && "Cannot clip boundary quad with interface");
	    auto f_out = mesh.add_face({it->second, my_verts[a2], my_verts[a3], jt->second});
	    dom_id[f_out] = domain_id.second;
	  }
      }
    // 2-in, 2-out, not successive
    else 
      {
	assert(n_in_verts==2);
	const int a0 = in_verts[0];       // in
	const int a1 = (in_verts[0]+1)%4; // out
	const int a2 = (in_verts[0]+2)%4; // in
	const int a3 = (in_verts[0]+3)%4; // out

	const auto e0 = mesh.find_edge(my_verts[a0], my_verts[a1]);
	const auto e1 = mesh.find_edge(my_verts[a1], my_verts[a2]);
	const auto e2 = mesh.find_edge(my_verts[a2], my_verts[a3]);
	const auto e3 = mesh.find_edge(my_verts[a3], my_verts[a0]);

	// new vertices along the 4 edges
	auto it0 = cutedgesMap.find(e0);
	auto it1 = cutedgesMap.find(e1);
	auto it2 = cutedgesMap.find(e2);
	auto it3 = cutedgesMap.find(e3);
	assert(it0!=cutedgesMap.end() && it1!=cutedgesMap.end() && it2!=cutedgesMap.end() && it3!=cutedgesMap.end());

	// erase the old face
	mesh.delete_face(e);

	// at most two vertices should have been deleted
	assert(nverts-mesh.n_vertices()<=2);

	// add new inner face
	auto f_in = mesh.add_face({my_verts[a0], it0->second, it1->second, my_verts[a2], it2->second, it3->second});
	dom_id[f_in] = domain_id.first;

	// add new outer faces
	if(discard_outer==false)
	  {
	    // no vertex should be deleted when erasing the face
	    assert(nverts-mesh.n_vertices()==0 && "Cannot clip boundary quad with interface");
	    auto f_out1 = mesh.add_face({it0->second, my_verts[a1], it1->second});
	    auto f_out2 = mesh.add_face({it2->second, my_verts[a3], it3->second});
	    dom_id[f_out1] = domain_id.second;
	    dom_id[f_out2] = domain_id.second;
	  }
      }
    
    
    // done
    return;
  }


  pmp::SurfaceMesh rebuild_mesh(const pmp::SurfaceMesh& mesh)
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

    // material id
    renum_mesh.add_face_property<int>("domain_id", 1);
      
    // transfer interface ids from the old mesh
    auto renum_interface_id = renum_mesh.add_vertex_property<int>("interface_id", -1);
    vcount = 0;
    for(auto v:v_container)
      {
	renum_interface_id[new_vertices[vcount]] = 1;
	++vcount;
      }
      
    // done
    return renum_mesh;
  }

}

