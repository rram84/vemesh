// Sriramajayam

#include <vm_MeshSlicer.h>

namespace vm
{
  namespace slicer
  {
    void prep_mesh(pmp::SurfaceMesh& mesh, const double phi_eps, LevelSetFunction_t& lsfunc,
		   const bool discard_outer_faces,
		   std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
		   std::map<pmp::Face, std::vector<int>>& cutfacesMap)
    {
      assert(phi_eps>0.);
      
      // cut edges -> new vertex map
      cutedgesMap.clear();
    
      // cut faces -> # of vertices in phi<0
      cutfacesMap.clear();

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
      auto f_container = mesh.faces();
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

	  if(n_in_verts>0 && n_out_verts>0) 	               // cut face
	    cutfacesMap.insert({f, in_verts});
	  else if(discard_outer_faces==true && n_in_verts==0)  // discard outside face
	    mesh.delete_face(f);
	}

      // should have at least some faces left
      if(discard_outer_faces==true)
	assert(mesh.n_faces()>0 && "Mesh does not intersect subzero level set");

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
	      cutedgesMap.insert({e, mesh.add_vertex(pmp::Point(y[0],y[1],y[2]))});
	    }
	}

      mesh.remove_vertex_property(lsvalues);
      assert(mesh.has_vertex_property("level set values")==false);

      // done
      return;
    }


    void slice_triangle(pmp::SurfaceMesh& mesh,
		       const std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
		       const pmp::Face& e, const std::vector<int>& in_verts,
		       const bool discard_outer)
    {
      const int n_in_verts = static_cast<int>(in_verts.size());
      assert(n_in_verts==1 || n_in_verts==2);
    
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
	  mesh.add_face({my_verts[a0], it->second, jt->second});

	  // create new outer face
	  if(discard_outer==false)
	    {
	      // no vertex can be deleted 
	      assert(mesh.n_vertices()==nverts && "Cannot clip boundary triangle with interface");
	      mesh.add_face({it->second, my_verts[a1], my_verts[a2], jt->second});
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
	  mesh.add_face({it->second, my_verts[a1], my_verts[a2], jt->second});

	  // create new outer face
	  if(discard_outer==false)
	    {
	      // no vertex can be deleted 
	      assert(mesh.n_vertices()==nverts && "Cannot clip boundary triangle with interface");
	      mesh.add_face({my_verts[a0], it->second, jt->second});
	    }
	}

      // done
      return;
    }



    void slice_quad(pmp::SurfaceMesh& mesh,
		    const std::map<pmp::Edge, pmp::Vertex>& cutedgesMap,
		    const pmp::Face& e,  std::vector<int> in_verts,
		    const bool discard_outer)
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
	  mesh.add_face({my_verts[a0], it->second, jt->second});

	  // create new outer face
	  if(discard_outer==false)
	    {
	      // no vertex should be deleted when erasing the face
	      assert(nverts==mesh.n_vertices() && "Cannot clip boundary quad with interface");
	      mesh.add_face({it->second, my_verts[a1], my_verts[a2], my_verts[a3], jt->second});
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
	  mesh.add_face({it->second, my_verts[a1], my_verts[a2], my_verts[a3], jt->second});

	  // create new outer face
	  if(discard_outer==false)
	    {
	      // no vertex should be deleted when erasing the face
	      assert(nverts-mesh.n_vertices()==0 && "Cannot clip boundary quad with interface");
	      mesh.add_face({it->second, jt->second, my_verts[a0]});
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
	  mesh.add_face({it->second, jt->second, my_verts[a0], my_verts[a1]});

	  // add new outer face
	  if(discard_outer==false)
	    {
	      // no vertex should be deleted when erasing the face
	      assert(nverts-mesh.n_vertices()==0 && "Cannot clip boundary quad with interface");
	      mesh.add_face({it->second, my_verts[a2], my_verts[a3], jt->second});
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
	  mesh.add_face({my_verts[a0], it0->second, it1->second, my_verts[a2], it2->second, it3->second});

	  // add new outer faces
	  if(discard_outer==false)
	    {
	      // no vertex should be deleted when erasing the face
	      assert(nverts-mesh.n_vertices()==0 && "Cannot clip boundary quad with interface");
	      mesh.add_face({it0->second, my_verts[a1], it1->second});
	      mesh.add_face({it2->second, my_verts[a3], it3->second});
	    }
	}
    
    
      // done
      return;
    }
    
  }
}
