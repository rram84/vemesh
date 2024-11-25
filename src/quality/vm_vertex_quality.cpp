// Sriramajayam

#include <vm_quality.h>
#include <cmath>
#include <limits>

namespace vm {

  // measure the quality of a face as the ratio of the area/perimeter^2
  double shape_quality(const pmp::SurfaceMesh& mesh, const pmp::Face& face) {

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

  
  // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
  // ratio of the area to the perimeter^2
  double VertexQuality::shape(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert) {

    // loop over incident faces
    // return the smallest quality among them
    double quality = std::numeric_limits<double>::max();
    auto f_circulator = mesh.faces(vert);
    for(auto f:f_circulator)
      {
  	double min_quality = shape_quality(mesh, f);
	if(min_quality<quality)
	  quality = min_quality;
      }
    return quality;
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
  double angle_quality(const pmp::SurfaceMesh& mesh, const pmp::Face& face)
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
  

  
  // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
  // minimum included angle
  double VertexQuality::angle(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert) {

    // loop over incident faces
    // return the smallest quality among them
    double quality = std::numeric_limits<double>::max();
    auto f_circulator = mesh.faces(vert);
    for(auto f:f_circulator)
      {
  	double min_quality = angle_quality(mesh, f);
	if(min_quality<quality)
	  quality = min_quality;
      }
    return quality;
  }
  

   // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
  // smallest nonzero eigenvalue
  double VertexQuality::stiffness(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert)
  {
    // loop over incident faces
    // return the smallest eigenvalue encountered
    double quality = std::numeric_limits<double>::max();
    auto f_circulator = mesh.faces(vert);
    for(auto f:f_circulator)
      {
  	double min_eigval = MeshFaceQuality_f(mesh, f, FaceQuality::stiffness);
	if(min_eigval<quality)
	  quality = min_eigval;
      }
    return quality;
  }
  
}
