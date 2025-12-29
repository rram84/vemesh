// Sriramajayam

#include <vm_tutorial_mesh_slicer.h>
#include <vm_tutorial_rectangle_mesh.h>
#include <vm_io.h>

double level_set_circle(const double* X)
{
  const double center[] {0.,0.};
  const double rad = 0.25;
  double Y[] = {X[0]-center[0], X[1]-center[1]};
  double r = std::sqrt(Y[0]*Y[0]+Y[1]*Y[1]);
  return r-rad;
}


int main()
{
  // rectangle mesh parameters
  const double left_cnr[] = {-0.5,-0.5};
  const int nx = 15;
  const int ny = 15;
  const double hx = 1./static_cast<double>(nx-1);
  const double hy = 1./static_cast<double>(ny-1);

  // circle
  vm::tutorial::LevelSetFunction_t lsfunc = level_set_circle; 
  const double phi_eps = 1.e-4;
  /*
  // clip
  auto mesh1 = vm::tutorial::create_rect_mesh(left_cnr, hx, nx, hy, ny, 0);
  vm::test::clip_mesh(mesh1, phi_eps, lsfunc, 1, 10);
  vm::write_off(mesh1, "quad-clip.OFF");
  vm::write_vtk(mesh1, "quad-clip.vtk");

  // embed
  auto mesh2 = vm::tutorial::create_rect_mesh(left_cnr, hx, nx, hy, ny, 0);
  vm::test::embed_interface(mesh2, phi_eps, lsfunc, {1,2}, 10);
  vm::write_off(mesh2, "quad-embed.OFF");
  vm::write_vtk(mesh2, "quad-embed.vtk");
  */
}
