// Sriramajayam

#include <vm_test_mesh_slicer.h>
#include <vm_io.h>


double level_set_circle(const double* X)
{
  const double center[] {0.5,0.5};
  const double rad = 0.25;
  double Y[] = {X[0]-center[0], X[1]-center[1]};
  double r = std::sqrt(Y[0]*Y[0]+Y[1]*Y[1]);
  return r-rad;
}

int main()
{
  vm::test::LevelSetFunction_t lsfunc = level_set_circle;
  const double phi_eps = 1.e-5;
  const double pert_eps = phi_eps/10.;

  // clip
  pmp::SurfaceMesh mesh1;
  vm::read_off("random_triangles.OFF", mesh1);
  vm::test::clip_mesh(mesh1, phi_eps, lsfunc); 
  vm::write_off(mesh1, "tri-clip.OFF"); 
  
  // embed
  pmp::SurfaceMesh mesh2;
  vm::read_off("random_triangles.OFF", mesh2);
  vm::test::embed_interface(mesh2, phi_eps, lsfunc, {1,2}); 
  vm::write_off(mesh2, "tri-embed.OFF");
  vm::write_vtk(mesh2, "tri-embed.vtk");
}
