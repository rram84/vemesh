// Sriramajayam

#include <vm_polygon_sampling.h>
#include <fstream>

int main()
{
  // polygon vertices
  std::vector<std::pair<double,double>>
    vertices{{2.0, 1.3}, {2.4, 1.7}, {2.8, 1.8}, {3.7, 1.6}, {3.4, 2.0}, {4.1, 3.0}, {5.3, 2.6}, {5.4, 1.2}, {4.9, 0.8}, {2.9, 0.7}};

  // compute a random sampling of 10 points
  auto samples = vm::compute_polygon_sampling(vertices, 10);

  // print polygon
  std::fstream pfile;
  pfile.open("poly.dat", std::ios::out);
  for(auto& v:vertices)
    pfile << v.first << " " << v.second << std::endl;
  pfile << vertices.front().first << " " << vertices.front().second;
  pfile.close();

  // print samples
  pfile.open("samples.dat", std::ios::out);
  for(auto& p:samples)
    pfile << p.first << " " << p.second << std::endl;
  pfile.close();
}
	 
