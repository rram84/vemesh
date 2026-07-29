// Sriramajayam

#include <vm_quality.h>
#include <iostream>
#include <fstream>

int main()
{
  std::fstream pfile;
  pfile.open("tri-contour.dat", std::ios::out);
  const int N = 100;
  std::vector<std::vector<double>> sigma(N+1, std::vector<double>(N+1));
  std::vector<std::vector<double>> shape(N+1, std::vector<double>(N+1));
  const double dx = 2.0/static_cast<double>(N);
  const double dy = 4.0/static_cast<double>(N);
  
  for(int i=0; i<=N; ++i)
    {
      std::cout << "i  = " << i << " of "<< N << std::endl;
      float x = static_cast<double>(i)*dx;
      
      for(int j=0; j<=N; ++j) {
	float y = 1.e-3+static_cast<double>(j)*dy;

	pmp::SurfaceMesh mesh;
	std::vector<pmp::Vertex> vertices{};
	vertices.push_back(mesh.add_vertex(pmp::Point({-1.,0.,0.})));
	vertices.push_back(mesh.add_vertex(pmp::Point({1.,0.,0.})));
	vertices.push_back(mesh.add_vertex(pmp::Point({x,y,0.})));
      
	auto quad = mesh.add_face(vertices);

	// compute face quality
	sigma[i][j] = vm::MeshFaceQuality_f(mesh, quad, vm::FaceQuality::stiffness);
	shape[i][j] = vm::VertexQuality::shape(mesh, vertices[3]);
	pfile << x << " " << y << " " << sigma[i][j] << " " << shape[i][j] << std::endl;
      }
      pfile << std::endl;
    }
  pfile.close();

  pfile.open("tri-dcontour.dat", std::ios::out);
  std::vector<std::vector<double>> dsigma(N, std::vector<double>(N));
  std::vector<std::vector<double>> dshape(N, std::vector<double>(N));
  for(int i=0; i<N; ++i)
    {
      double x = static_cast<double>(i)*dx;
      for(int j=0; j<N; ++j)
	{
	  double y = 1.e-3+static_cast<double>(j)*dy;
	  double dsigma[2] = {(sigma[i+1][j]-sigma[i][j])/dx, (sigma[i][j+1]-sigma[i][j])/dy};
	  double dshape[2] = {(shape[i+1][j]-shape[i][j])/dx, (shape[i][j+1]-shape[i][j])/dy};

	  pfile << x << " " << y << " " << dsigma[0] << " " << dsigma[1] << " " << dshape[0] << " " << dshape[1] << std::endl;
	}
    }
      pfile.close();
}
