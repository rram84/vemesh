// Sriramajayam

#include <vm_SpecialMeshes.h>
#include <vm_io.h>
int main()
{
  // delaunay triangulation
  std::vector<std::pair<double,double>> points{{1.,1.}, {1.,2.}, {1.,3.}, {2.,1.}, {2.,2.}, {2.,3.}};
  auto mesh_1 = vm::create_delaunay_triangulation(points);
  vm::write_off(mesh_1, "del_tri.OFF");


  // delaunay triangulation of a random collection of points
  const double bot_left_cnr[]  = {-1.,0.};
  const double top_right_cnr[] = {2.,3.};
  const int num_points = 150;
  auto mesh_2 = vm::create_random_delaunay(bot_left_cnr, top_right_cnr, num_points);
  vm::write_off(mesh_2, "random_tri.OFF");
}

