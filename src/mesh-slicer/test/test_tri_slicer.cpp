// Sriramajayam

#include <vm_MeshSlicer.h>
#include <vm_io.h>


double level_set_circle(const double* X)
{
  const double center[] {0.5,0.5};
  const double rad = 0.35;
  double Y[] = {X[0]-center[0], X[1]-center[1]};
  double r = std::sqrt(Y[0]*Y[0]+Y[1]*Y[1]);
  return r-rad;
}

int main()
{
  pmp::SurfaceMesh mesh;
  vm::read_off("random_triangles.OFF", mesh);
  vm::LevelSetFunction_t lsfunc = level_set_circle;
  vm::clip_mesh(mesh, 0.001, lsfunc);
  vm::write_off(mesh, "ls.OFF");
}
