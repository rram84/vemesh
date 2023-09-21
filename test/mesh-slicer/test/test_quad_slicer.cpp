// Sriramajayam

#include <vm_test_mesh_slicer.h>
#include <vm_test_rectangle_mesh.h>
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
  vm::test::LevelSetFunction_t lsfunc = level_set_circle; 
  const double phi_eps = 1.e-4;
  
  // clip
  auto mesh1 = vm::test::create_rect_mesh(left_cnr, hx, nx, hy, ny);
  vm::test::clip_mesh(mesh1, phi_eps, lsfunc);
  vm::write_off(mesh1, "quad-clip.OFF");

  // embed
  auto mesh2 = vm::test::create_rect_mesh(left_cnr, hx, nx, hy, ny);
  vm::test::embed_interface(mesh2, phi_eps, lsfunc, {1,2});
  vm::write_off(mesh2, "quad-embed.OFF");
  vm::write_vtk(mesh2, "quad-embed.vtk");
}
