// Sriramajayam

#include <vm_Manager.h>
#include <vm_polygon_kernel.h>
#include <iostream>

namespace vm
{
  // compute the vertex ring
  std::vector<pmp::Vertex> get_vertex_ring(const pmp::SurfaceMesh& mesh, const pmp::Vertex& v)
  {
    // list to return
    std::vector<pmp::Vertex> vertex_ring{};
    
    // faces incident at v
    auto face_circulator = mesh.faces(v);

    for(auto f:face_circulator)
      {
	// vertices of this face
	auto vert_circulator = mesh.vertices(f);
	std::vector<pmp::Vertex> face_vertices{};
	int v_indx = -1;
	int indx = 0;
	for(auto w:vert_circulator)
	  {
	    face_vertices.push_back(w);
	    if(w.idx()==v.idx())
	      v_indx = indx;
	    ++indx;
	    std::cout << " " << w.idx();
	  }
	std::cout << std::endl;
	const int nVerts = static_cast<int>(face_vertices.size());
	
	// permute the list of face vertices until 'v' appears first
	std::rotate(face_vertices.begin(), face_vertices.begin()+v_indx, face_vertices.end());
	assert(face_vertices.front().idx()==v.idx());

	// append vertices to the ring, exclude 'v' and repetitions
	for(int i=2; i<nVerts; ++i)
	  vertex_ring.push_back(face_vertices[i]);
      }

    // done
    return std::move(vertex_ring);
  }
  
  
  // move vertices to kernel centers
  void Manager::move_to_kernel()
  {
    // move vertices not lying on the boundary to the kernel of the polygon around it
    auto vertices = mesh.vertices();
    for(auto v:vertices)
      if(mesh.is_boundary(v)==false)
	{
	  std::cout << "Moving vertex " << v.idx() << std::endl;
	  auto nb_verts = get_vertex_ring(mesh, v); //mesh.vertices(v);
	  std::cout << "1-ring vertices: ";
	  for(auto w:nb_verts)
	    std::cout << w.idx() <<" " ;
	  std::cout << std::endl;
	  std::vector<pmp::Point> nb_vertex_coords{};
	  for(auto w:nb_verts)
	    nb_vertex_coords.push_back(mesh.position(w));

	  // kernel of the polygon formed by vertex neighbors
	  auto kernel_verts = compute_polygon_kernel(nb_vertex_coords);

	  // centroid of the kernel
	  double center[2] = {0.,0.};
	  const int num_kernel_verts = static_cast<int>(kernel_verts.size());
	  for(auto& it:kernel_verts)
	    {
	      center[0] += it.first;
	      center[1] += it.second;
	    }
	  center[0] /= static_cast<double>(num_kernel_verts);
	  center[1] /= static_cast<double>(num_kernel_verts);

	  // udpate the position of this vertex
	  pmp::Point& X = mesh.position(v);
	  X[0] = center[0];
	  X[1] = center[1];
	}

    // done
    return;
  }

  
}

