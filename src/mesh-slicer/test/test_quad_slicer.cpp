// Sriramajayam

#include <vm_MeshSlicer.h>
#include <vm_SpecialMeshes.h>
#include <vm_io.h>


double level_set_circle(const double* X)
{
  const double center[] {0.,0.};
  const double rad = 0.35;
  double Y[] = {X[0]-center[0], X[1]-center[1]};
  double r = std::sqrt(Y[0]*Y[0]+Y[1]*Y[1]);
  return r-rad;
}

int main()
{
  // rectangle mesh
  const double left_cnr[] = {-0.5,-0.5};
  const int nx = 15;
  const int ny = 15;
  const double hx = 1./static_cast<double>(nx-1);
  const double hy = 1./static_cast<double>(ny-1);
  auto mesh = vm::create_rect_mesh(left_cnr, hx, nx, hy, ny);
  vm::write_off(mesh, "rect.off");

  // clip
  vm::LevelSetFunction_t lsfunc = level_set_circle;
  vm::clip_mesh(mesh, lsfunc);
  vm::write_off(mesh, "ls.OFF");
}
