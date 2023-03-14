// Sriramajayam

#include <vm_MeshSlicer.h>
#include <cassert>
#include <map>

namespace vm
{
  pmp::SurfaceMesh renumber_mesh_vertices(const pmp::SurfaceMesh& mesh);
  
  void clip_triangle(pmp::SurfaceMesh& mesh,
		     const std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
		     const pmp::Face& e, const std::vector<int>& in_verts);

  void clip_quad(pmp::SurfaceMesh& mesh,
		 const std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
		 const pmp::Face& e,  std::vector<int> in_verts);

  
  void clip_mesh(pmp::SurfaceMesh& mesh, LevelSetFunction_t& lsfunc)
  {
    // cut edges -> new vertex map
    std::map<pmp::Edge, pmp::Vertex> cutedgesMap{};
    
    // cut faces -> local inner vertices map
    std::map<pmp::Face, std::vector<int>> cutfacesMap{};

    // remove outer faces
    // insert vertices along cut edges
    // identify cut faces, and their local inner vertices
    // check that this is a triangle/quad mesh
    
    // compute the level set function at the nodes
    auto lsvalues    = mesh.add_vertex_property<double>("level set values");
    auto v_container = mesh.vertices();
    double Y[2];
    for(auto v:v_container)
      {
	const auto& X = mesh.position(v);
	Y[0] = X[0];
	Y[1] = X[1];
	lsvalues[v] = lsfunc(Y);
      }

    // remove faces that lie completely outside
    // populate list of faces that need to be clipped
    auto f_container = mesh.faces();
    cutfacesMap.clear();
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

	// no vertices inside? erase face
	const int num_verts = mesh.valence(f);
	if(in_verts.empty())             
	  mesh.delete_face(f);
	// at least one vertex outside
	else if(static_cast<int>(in_verts.size())<num_verts)
	  cutfacesMap.insert({f, in_verts});
      }
    
    // compute locations of new vertices on a per-edge basis
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
	    cutedgesMap.insert({e, mesh.add_vertex(pmp::Point(y[0],y[1],y[2]))});
	  }
      }

    mesh.remove_vertex_property(lsvalues);
    assert(mesh.has_vertex_property("level set values")==false);
    
    // clip triangles and quads
    for(auto& it:cutfacesMap)
      {
	// triangle to clip and its local inner vertices
	const auto& e        = it.first;
	const auto& in_verts = it.second;
	const int n_verts    = mesh.valence(e);
	assert(n_verts==3 || n_verts==4);
	
	// clip a triangle/quad
	if(n_verts==3)
	  clip_triangle(mesh, cutedgesMap, e, in_verts);
	else 
	  clip_quad(mesh, cutedgesMap, e, in_verts);
      }
    
    // renumber the mesh
    mesh = renumber_mesh_vertices(mesh);
     
    // done
    return;
  }


  
  void clip_triangle(pmp::SurfaceMesh& mesh,
		     const std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
		     const pmp::Face& e, const std::vector<int>& in_verts)
  {
    const int n_in_verts = static_cast<int>(in_verts.size());
    assert(n_in_verts==1 || n_in_verts==2);
    
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
	
	// create new face
	mesh.add_face({it->second, my_verts[a1], my_verts[a2], jt->second});
      }

    // done
    return;
  }


  void clip_quad(pmp::SurfaceMesh& mesh,
		 const std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
		 const pmp::Face& e,  std::vector<int> in_verts)
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
	const auto e0 = mesh.find_edge(my_verts[a1], my_verts[a2]);
	const auto e1 = mesh.find_edge(my_verts[a3], my_verts[a0]);
	
	// vertices inserted along e0 and e1
	auto it = cutedgesMap.find(e0);
	auto jt = cutedgesMap.find(e1);
	assert(it!=cutedgesMap.end() && jt!=cutedgesMap.end());
	
	// erase the old face
	mesh.delete_face(e);
	
	// add new face
	mesh.add_face({it->second, jt->second, my_verts[a0], my_verts[a1]});
      }
    
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
  
  
  void clip_mesh_prep(pmp::SurfaceMesh& mesh, const int num_verts_per_face,
		      LevelSetFunction_t& lsfunc,
		      std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
		      std::map<pmp::Face, std::vector<int>>& cutfacesMap)
  {
    // done
    return;
  }
		      
}
