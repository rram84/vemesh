// Sriramajayam

#include <vm_vertex_ring.h>
#include <vm_io.h>

int main()
{
  pmp::SurfaceMesh mesh;
  vm::read_off("ring.off", mesh);
  vm::write_vtk(mesh, "ring.vtk");
		
  // compute the vertex ring
  pmp::Vertex v0(0);
  auto vring = vm::get_vertex_ring(mesh, v0);
  std::cout << "Vertex ring: ";
  for(auto& v:vring)
    std::cout << v.idx() << " " ;
			    std::cout << std::endl;
}
