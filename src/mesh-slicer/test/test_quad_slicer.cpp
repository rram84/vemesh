// Sriramajayam

#include <vm_MeshSlicer.h>
#include <vm_SpecialMeshes.h>
#include <vm_io.h>
#include <random>

// circle radius
double rad = 0.0;



double level_set_circle(const double* X)
{
  const double center[] {0.,0.};
  //const double rad = 0.35;
  double Y[] = {X[0]-center[0], X[1]-center[1]};
  double r = std::sqrt(Y[0]*Y[0]+Y[1]*Y[1]);
  return r-rad;
}

double level_set_segment(const double* X)
{
  const double yseg = 0.2859;
  return X[1]-yseg;
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

  // random circle radius
  std::random_device rd; 
  std::mt19937 gen(rd()); 
  std::uniform_real_distribution<> dis(0.2,0.4);
  rad = dis(gen);
    
  // clip
  vm::LevelSetFunction_t lsfunc = level_set_segment;  //level_set_circle; 
  vm::clip_mesh(mesh, lsfunc);
  vm::write_off(mesh, "ls-0p2859.OFF");
  vm::write_suku_format(mesh, "ls-0p2859");
}
