// Sriramajayam

#include <vm_test_rectangle_mesh.h>
#include <cassert>

namespace vm
{
  namespace test
  {
    pmp::SurfaceMesh create_rect_mesh(const double* left_cnr,
				      const double hx, const int nx,
				      const double hy, const int ny)
    {
      assert(nx>1 && ny>1);
      pmp::SurfaceMesh mesh;

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
    
      // done
      return mesh;
    }
  }    
}

