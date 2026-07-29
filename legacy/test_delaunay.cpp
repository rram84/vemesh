// Sriramajayam

#include <vm_SpecialMeshes.h>
#include <vm_io.h>
#include <random>
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
  std::random_device rd;  
  std::mt19937 gen(rd()); 
  std::uniform_real_distribution<> xdis(bot_left_cnr[0], top_right_cnr[0]);
  std::uniform_real_distribution<> ydis(bot_left_cnr[1], top_right_cnr[1]);
  std::vector<std::pair<double,double>> rand_points(num_points);
  for(int i=0; i<num_points; ++i)
    rand_points[i] = {xdis(gen), ydis(gen)};
  auto mesh_2 = vm::create_delaunay_triangulation(rand_points);
  vm::write_off(mesh_2, "random_tri.OFF");
}

