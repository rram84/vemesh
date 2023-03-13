// Sriramajayam

#include <vm_MeshSlicer.h>
#include <cassert>
#include <map>

namespace vm
{
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
    // check that this is a triangle/quad mesh
    assert(num_verts_per_face==3 || num_verts_per_face==4);
    if(num_verts_per_face==3)
      assert(mesh.is_triangle_mesh()==true);
    else
      assert(mesh.is_quad_mesh()==true);

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
	if(in_verts.empty())             
	  mesh.delete_face(f);
	// at least one vertex outside
	else if(static_cast<int>(in_verts.size())<num_verts_per_face) 
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

    // done
    return;
  }
		      
}
