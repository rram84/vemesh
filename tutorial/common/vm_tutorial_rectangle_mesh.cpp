// Sriramajayam

/** \file vm_tutorial_rectangle_mesh.cpp
 * \brief Implementation of the rectangle/quad mesh generation utility
 * \author Ramsharan Rangarajan
 */

#include <vm_tutorial_rectangle_mesh.h>
#include <cassert>

namespace vm
{
  namespace tutorial
  {
    pmp::SurfaceMesh create_rectangle_mesh(const std::array<double,2> left_cnr,
					   const double hx, const int nx,
					   const double hy, const int ny)
    {
      if (nx <= 1 || ny <= 1)
	throw std::runtime_error("create_rectangle_mesh: nx and ny must be > 1");

      pmp::SurfaceMesh mesh;
      const int domain_id = 0;
      
      // add vertices row-by-row
      std::vector<pmp::Vertex> vertices(nx*ny);
      for(int j=0; j<ny; ++j)
	{
	  const double ycoord = left_cnr[1] + static_cast<double>(j)*hy;
	  for(int i=0; i<nx; ++i)
	    {
	      const double xcoord = left_cnr[0] + static_cast<double>(i)*hx;
	      vertices[nx*j+i] = mesh.add_vertex(pmp::Point(xcoord, ycoord, 0.));
	    }
	}

      // add faces row by row
      for(int j=0; j<ny-1; ++j)
	for(int i=0; i<nx-1; ++i)
	  mesh.add_face({vertices[nx*j+i], vertices[nx*j+i+1], vertices[nx*(j+1)+i+1], vertices[nx*(j+1)+i]});

      assert(mesh.n_vertices()==nx*ny);
      assert(mesh.n_faces()==(nx-1)*(ny-1));

      // assign material id
      mesh.add_face_property<int>("domain_id", domain_id);
      
      // no vertices on an interface
      mesh.add_vertex_property<int>("interface_id", -1);
      
      // done
      return mesh;
    }
  }    
}

