// Sriramajayam

#include <vm_quality.h>
#include <vm_utils.h>
#include <cmath>

namespace vm {

  // measure the quality of a face as the ratio of the area/perimeter^2
  double FaceQuality::shape(const pmp::SurfaceMesh& mesh, const pmp::Face& face) {

    // boost polygon of this face
    boost_polygon_t poly;
    auto v_circulator = mesh.vertices(face);
    for(auto v:v_circulator) {
      const auto& X = mesh.position(v);
      bg::append(poly.outer(), boost_point_t(X[0],X[1]));
    }
    auto first_vertex = *poly.outer().begin();
    bg::append(poly.outer(), first_vertex);

    // area
    double area = bg::area(poly);
    double perim = bg::perimeter(poly);

    // normalizing factor for this polygon
    const double n = static_cast<double>(mesh.valence(face));
    const double factor = 4.*n*std::tan(M_PI/n);
    return factor*area/(perim*perim);
  }


    // compute the angle included by the pair of segments joining three points
  /*
   * u    w
   * \  /
   *  v
   */
  double compute_included_angle_in_degrees(const pmp::Point& U, const pmp::Point& V, const pmp::Point& W)
  {
    // edges
    const double VU[] = {U[0]-V[0], U[1]-V[1]};
    const double VW[] = {W[0]-V[0], W[1]-V[1]};

    // measure the angle at vertex V
    const double dot = VU[0]*VW[0] + VU[1]*VW[1];
    const double det = VU[0]*VW[1] - VU[1]*VW[0];
    double angle     = std::atan2(-det, dot);
    if(angle<0.)
      angle += 2.*M_PI;

    return (180./M_PI)*angle;
  }
  
  // measure quality of a face as the smallest included angle
  double FaceQuality::angle(const pmp::SurfaceMesh& mesh, const pmp::Face& face)
  {
    // make a list of vertex coordinates
    std::vector<pmp::Point> coords{};
    auto v_circulator = mesh.vertices(face);
    for(auto v:v_circulator)
      coords.push_back( mesh.position(v) );

    const int nverts = static_cast<int>(coords.size());
    double min_angle = 360.;
    for(int a=0; a<nverts; ++a)
      {
	const auto& Xa = coords[a];
	const auto& Xb = coords[(a+1)%nverts];
	const auto& Xc = coords[(a+2)%nverts];

	// angle between edges ab and bc
	const double angle = compute_included_angle_in_degrees(Xa, Xb, Xc);
	
	// track the minimum
	if(angle<min_angle)
	  min_angle = angle;
      }
    
    return min_angle;
  }
  
  
  // measure quality of a face as the smallest nonzero eigenvalue of the vem stiffness matrix
  double FaceQuality::stiffness(const pmp::SurfaceMesh& mesh, const pmp::Face& face)
  {
    // vertex coordinates
    std::vector<pmp::Point> coords{};
    auto v_circulator = mesh.vertices(face);
    for(auto v:v_circulator)
      coords.push_back(mesh.position(v));

    return FaceQuality::stiffness(coords);
  }


  double FaceQuality::stiffness(const std::vector<pmp::Point>& coords)
  {
    // stiffness matrix
    auto Kmat = compute_polygon_stiffness_matrix(coords, 1.0);
    
    // eigenvalues
    auto eigvals = Kmat.selfadjointView<Eigen::Lower>().eigenvalues();
    const int nvals = eigvals.rows();
    auto* eigarray = eigvals.data();
    std::sort(eigarray, eigarray+nvals);

    // first value should be approximately zero, subsequent should be positive
    assert(eigarray[1]>eigarray[0]);

    // return the second eigenvalue
    return eigarray[1]/eigarray[nvals-1];
  }

}
