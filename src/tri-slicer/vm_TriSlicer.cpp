// Sriramajayam

#include <vm_TriSlicer.h>
#include <cassert>
#include <list>
#include <map>
#include <iostream>

namespace vm
{
  pmp::SurfaceMesh renumber_mesh_vertices(const pmp::SurfaceMesh& mesh);
  
  void clip_tri_mesh(pmp::SurfaceMesh& mesh, LevelSetFunction_t& lsfunc)
  {
    // check that this is a triangle mesh
    assert(mesh.is_triangle_mesh()==true);

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

    // remove triangles that lie completely outside
    auto f_container = mesh.faces();
    for(auto f:f_container)
      {
	int n_plus = 0;
	auto v_circulator = mesh.vertices(f);
	for(auto v:v_circulator)
	  {
	    if(lsvalues[v]>=0.)
	      ++n_plus;
	  }
	if(n_plus==3)
	  mesh.delete_face(f);
      }

    // triangles that need to be clipped
    std::list<pmp::Face> clip_triangles{};
    f_container = mesh.faces();
    for(auto f:f_container)
      {
	int n_plus  = 0;
	auto v_circulator = mesh.vertices(f);
	for(auto v:v_circulator)
	  {
	    if(lsvalues[v]>=0.)
	      ++n_plus;
	  }
	if(n_plus>0)
	  clip_triangles.push_back(f);
      }
    
    // compute location of new vertex on a per-edge basis
    std::map<pmp::Edge, pmp::Vertex> split_edge_map{};
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
	    split_edge_map.insert({e, mesh.add_vertex(pmp::Point(y[0],y[1],y[2]))});
	  }
      }

    // clip triangles
    while(clip_triangles.empty()==false)
      {
	// triangle to clip
	auto e = clip_triangles.front();
	clip_triangles.pop_front();

	auto v_circulator = mesh.vertices(e);

	// vertices of this face
	std::vector<pmp::Vertex> my_verts{};

	// local vertices on the inside/outside
	std::vector<int> in_verts{}, out_verts{};
	int a = 0;

	for(auto v:v_circulator)
	  {
	    my_verts.push_back(v);
	    if(lsvalues[v]<0.)
	      in_verts.push_back(a);
	    else
	      out_verts.push_back(a);
	    ++a;
	  }

	// 1-in, 2-out case
	if(static_cast<int>(in_verts.size())==1)
	  {
	    // edges emanating from the inner vertex
	    const int a0 = in_verts[0];
	    const int a1 = (a0+1)%3;
	    const int a2 = (a1+1)%3;
	    const auto e0 = mesh.find_edge(my_verts[a0], my_verts[a1]);
	    const auto e1 = mesh.find_edge(my_verts[a2], my_verts[a0]);

	    // vertices inserted along e0 and e1
	    auto it = split_edge_map.find(e0);
	    auto jt = split_edge_map.find(e1);
	    assert(it!=split_edge_map.end() && jt!=split_edge_map.end());

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
	    auto it = split_edge_map.find(e0);
	    auto jt = split_edge_map.find(e1);
	    assert(it!=split_edge_map.end() && jt!=split_edge_map.end());

	    // erase the old face
	    mesh.delete_face(e);

	    // create new face
	    mesh.add_face({it->second, my_verts[a1], my_verts[a2], jt->second});
	  }
      }
	  
    mesh.remove_vertex_property(lsvalues);
    assert(mesh.has_vertex_property("level set values")==false);
     
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
