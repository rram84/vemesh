// Sriramajayam

#include <vm_test_rectangle_mesh.h>
#include <vm_io.h>

int main()
{
  const double left_cnr[] = {-0.5,-0.5};
  const int nx = 15;
  const int ny = 15;
  const double hx = 1./static_cast<double>(nx-1);
  const double hy = 1./static_cast<double>(ny-1);
  auto mesh = vm::test::create_rect_mesh(left_cnr, hx, nx, hy, ny);
  vm::write_off(mesh, "rect.off");
}
