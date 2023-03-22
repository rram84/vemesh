// Sriramajayam

#include <vm_polygon_stiffness.h>

// boost polygon utilities
#include <boost/geometry/geometry.hpp>

namespace vm
{
  // boost aliases
  namespace bg  = boost::geometry;
  namespace bgm = bg::model;
  using boost_point_t         = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_polygon_t       = bgm::polygon<boost_point_t, false>;

  // distance between farthest vertices
  double compute_polygon_dia(const std::vector<std::array<double,2>>& coords);

  // centroid of the polygon
  std::array<double,2> compute_polygon_centroid(const std::vector<std::array<double,2>>& coords);
  
  // area of the polygon
  double compute_polygon_area(const std::vector<std::array<double,2>>& coords);

  // normals at vertices
  std::vector<std::array<double,2>> compute_vertex_normals(const std::vector<std::array<double,2>>& coords);

  Eigen::MatrixXd compute_polygon_stiffness_matrix(const std::vector<std::array<double,2>>& coords, const double tau)
  {
    // # vertices
    const int nverts = static_cast<int>(coords.size());
    
    // farthest two points
    const double hE = compute_polygon_dia(coords);
    
    // centroid
    const auto XE = compute_polygon_centroid(coords);
    
    // area
    const double area = compute_polygon_area(coords);
    
    // vertex normals
    const auto bvecs = compute_vertex_normals(coords);
 
    // G matrix
    Eigen::MatrixXd G = Eigen::MatrixXd::Zero(3,3);
    G(0,0) = 1.;
    G(1,1) = area/(hE*hE);
    G(2,2) = area/(hE*hE);
    
    // B matrix
    Eigen::MatrixXd B(3, nverts);
    for(int j=0; j<nverts; ++j)
      {
	B(0,j) = 1./static_cast<double>(nverts);
	B(1,j) = bvecs[j][0]/hE;
	B(2,j) = bvecs[j][1]/hE;
      }
    
    // Pi*_Delta matrix
    Eigen::MatrixXd Pi_star_delta = G.inverse()*B;

    // Delta matrix
    Eigen::MatrixXd Delta(nverts,3);
    for(int i=0; i<nverts; ++i)
      {
	Delta(i,0) = 1.;
	Delta(i,1) = (coords[i][0]-XE[0])/hE;
	Delta(i,2) = (coords[i][1]-XE[1])/hE;
      }

    // Pi^Delta matrix
    Eigen::MatrixXd Pi_delta = Delta*Pi_star_delta;

    // Modify G
    G(0,0) = 0.;

    // Identity
    Eigen::MatrixXd Id = Eigen::MatrixXd::Identity(nverts, nverts);

    // stiffness matrix
    return Pi_star_delta.transpose()*G*Pi_star_delta + tau*(Id-Pi_delta)*(Id-Pi_delta);
  }



  // distance between farthest vertices
  double compute_polygon_dia(const std::vector<std::array<double,2>>& coords)
  {
    const int nverts = static_cast<int>(coords.size());
    double dia = 0.;
    double dist2;
    for(int i=0; i<nverts; ++i)
      {
	const auto& X = coords[i];
	for(int j=i+1; j<nverts; ++j)
	  {
	    const auto& Y = coords[j];
	    dist2 = (X[0]-Y[0])*(X[0]-Y[0]) + (X[1]-Y[1])*(X[1]-Y[1]);
	    if(dist2>dia)
	      dia = dist2;
	  }
      }
    return std::sqrt(dia);
  }


  // centroid of the polygon
  std::array<double,2> compute_polygon_centroid(const std::vector<std::array<double,2>>& coords)
  {
    std::array<double,2> C{0.,0.};
    for(auto& X:coords)
      {
	C[0] += X[0];
	C[1] += X[1];
      }
    const double nverts = static_cast<double>(coords.size());
    C[0] /= nverts;
    C[1] /= nverts;
    return std::move(C);
  }

  
  // area of the polygon
  double compute_polygon_area(const std::vector<std::array<double,2>>& coords)
  {
    // create boost polygon
    boost_polygon_t poly;
    for(auto& X:coords)
      bg::append(poly.outer(), boost_point_t(X[0],X[1]));
    bg::append(poly.outer(), boost_point_t(coords[0][0], coords[0][1]));

    return bg::area(poly);
  }

  
  // vertex normals
  std::vector<std::array<double,2>> compute_vertex_normals(const std::vector<std::array<double,2>>& coords)
  {
    // average un-normalized edge vectors
    const int nverts = static_cast<int>(coords.size());
    std::vector<std::array<double,2>> bvecs(nverts);
    for(int i=1; i<=nverts; ++i)
      {
	// left/right vertices
	const auto& XL = coords[i-1];
	const auto& XR = coords[(i+1)%nverts];

	// average = vertex normal
	bvecs[i%nverts][0] = 0.5*(XL[1]-XR[1]);
	bvecs[i%nverts][1] = 0.5*(XR[0]-XL[0]);
      }

    return std::move(bvecs);
  }

}
