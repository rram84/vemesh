// Sriramajayam

#include <vm_io.h>

int main()
{
  {
    pmp::SurfaceMesh mesh;
    vm::read_off("bbbb-3.off", mesh);
    vm::write_vtk(mesh, "bbbb-3.vtk");
    exit(1);
  }
  
  pmp::SurfaceMesh mesh;
  vm::read_vtk("circ/embed-686.vtk", mesh);

  auto material_id = mesh.get_face_property<int>("material_id");
  auto cut_id = mesh.add_face_property<int>("cut_id", 0);
  auto f_container = mesh.faces();
  for(auto f:f_container)
    {
      auto my_id = material_id[f];
      auto hedges = mesh.halfedges(f);
      for(auto h:hedges)
	{
	  auto h_opp = mesh.opposite_halfedge(h);
	  if(!mesh.is_boundary(h_opp))
	    {
	      auto nb = mesh.face(h_opp);
	      auto nb_id = material_id[nb];
	      if(my_id!=nb_id)
		{
		  cut_id[f] = my_id;
		  cut_id[nb] = nb_id;
		}
	    }
	}
    }

  for(auto f:f_container)
    material_id[f] = cut_id[f];
  
  vm::write_vtk(mesh, "worst-fem-circ.vtk");
}
