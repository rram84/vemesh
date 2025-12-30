// Sriramajayam

#include <vm_mesh_inspection.h>
#include <sstream>

namespace vm
{
  namespace {
    
    // Helper to add error if optional list is provided
    void add_error(const std::optional<std::reference_wrapper<ErrorList>> &errors, InspectionError e) {
      if (errors) errors->get().push_back(std::move(e));
    }
  
    bool inspect_mesh_basic(const pmp::SurfaceMesh& mesh,
			    const std::optional<std::reference_wrapper<ErrorList>> &errors)
    {
      bool flag = true;
      if(mesh.n_vertices()==0 || mesh.n_faces()==0 || mesh.n_edges()==0)
	{
	  flag = false;
	  add_error(errors, {InspectionErrorCode::EmptyMesh,-1, -1, "Empty mesh"});
	}
	
      return flag;
    }
    
    bool inspect_mesh_faces(const std::map<int, boost_polygon_t>& polygons,
			    const std::optional<std::reference_wrapper<ErrorList>> &errors)
    {
      bool flag = true;
    
      // each face should be valid, simple and have a positive area
      for(const auto& [findx, poly]: polygons) {

	std::string message;
	if(!bg::is_valid(poly, message))
	  {
	    flag = false;
	    std::ostringstream oss;
	    oss << "Boost message: " << message << "\nFace coordinates: " << boost::geometry::wkt(poly) << "\n";
	    add_error(errors, {InspectionErrorCode::InvalidFace, findx, -1, oss.str()});
	  }
      
	if(!bg::is_simple(poly))
	  {
	    flag = false;
	    std::ostringstream oss;
	    oss << "Face coordinates: " << boost::geometry::wkt(poly) <<"\n";
	    add_error(errors, {InspectionErrorCode::NonSimpleFace, findx, -1, oss.str()});
	  }
      
	if(bg::area(poly)<=0.)
	  {
	    flag = false;
	    std::ostringstream oss;
	    oss <<"Face coordinates: " << boost::geometry::wkt(poly) << "\n";
	    add_error(errors, {InspectionErrorCode::NonPositiveArea, findx, -1, oss.str()});
	  }
      }
    
      return flag;
    }

  
    bool inspect_mesh_adjacency(const pmp::SurfaceMesh& mesh,
				const std::map<int, boost_polygon_t>& polygons,
				const std::optional<std::reference_wrapper<ErrorList>> &errors)
    {
      auto face_circulator = mesh.faces();
    
      // faces should not overlap with neighbors
      // check intersection with neighbors
      bool flag = true;
      for(auto face:face_circulator) {

	// this face
	const int& findx = face.idx();
	const auto& poly = polygons.at(findx);
      
	// area
	double area = bg::area(poly);
      
	// its neighbors
	auto halfedge_circulator = mesh.halfedges(face);
	for(auto h:halfedge_circulator)
	  {
	    assert(mesh.is_valid(h));
	    assert(mesh.is_boundary(h)==false);  	// cannot be a boundary halfedge

	    // nothing to do in case of no neighbor
	    auto h_opp  = mesh.opposite_halfedge(h);
	    if(mesh.is_boundary(h_opp)==true)
	      continue;
	  
	    // neighboring face
	    auto nb_face = mesh.face(h_opp);
	    assert(mesh.is_valid(nb_face)==true);

	    // avoid double checking pairwise insersections
	    const int nb_findx = nb_face.idx();
	    if(findx>nb_findx)
	      continue;

	    // check pairwise intersection
	    const auto& nb_poly = polygons.at(nb_findx);
	    boost_multi_polygon_t intersection;
	    bool does_intersect = bg::intersection(poly, nb_poly, intersection);
	    assert(does_intersect==true); // at vertices and edges
	    double intersection_area = bg::area(intersection);

	    if(std::abs(intersection_area/area)>1.e-3) {
	      flag = false;
	      std::ostringstream oss;
	      oss << "Face: " << boost::geometry::wkt(poly) << "\nNeighbor face: " << boost::geometry::wkt(nb_poly) << "\n";
	      add_error(errors, {InspectionErrorCode::FaceOverlap, findx, nb_findx, oss.str()});
	    }
	  }
      }
      return flag;
    }
  }

  bool inspect_mesh(const pmp::SurfaceMesh& mesh,
		    MeshInspection level,
		    std::optional<std::reference_wrapper<ErrorList>> errors)
  {
    // ---- Basic checks ---- //
    bool flag = inspect_mesh_basic(mesh, errors);
    if (flag==false || level==MeshInspection::Basic)
      return flag;
    
    // ---- Face geometry checks ---- //
    std::map<int, boost_polygon_t> polygons{};
    auto face_circulator = mesh.faces();
    for(auto face:face_circulator)
      {
	// vertices of this face
	auto vertex_circulator = mesh.vertices(face);
	
	// boost polygon representation
	std::vector<pmp::Point> coords{};
	for(auto v:vertex_circulator)
	  coords.push_back(mesh.position(v));
	boost_polygon_t poly = make_polygon(coords);
	
	polygons.insert({face.idx(), poly});
      }

    flag = inspect_mesh_faces(polygons, errors);
    if(flag==false || level==MeshInspection::FaceGeometry)
      return flag;
    
    // ---- Adjacency ---- //
    flag = inspect_mesh_adjacency(mesh, polygons, errors);
    return flag;
  }


  namespace {
    std::string to_string(const InspectionErrorCode code)
    {
      switch (code) {
      case InspectionErrorCode::EmptyMesh:       return "EmptyMesh";
      case InspectionErrorCode::InvalidFace:     return "InvalidFace";
      case InspectionErrorCode::NonSimpleFace:   return "NonSimpleFace";
      case InspectionErrorCode::NonPositiveArea: return "NonPositiveArea";
      case InspectionErrorCode::FaceOverlap:     return "FaceOverlap";
      default:                                   return "UnknownError";
      }
    }
  }
  
  std::ostream& operator<<(std::ostream& os, const InspectionError& e)
  {
    os << "Code: " << to_string(e.code)
       << ", Face: " << e.face;
    if (e.face2 != -1)
      os << ", Face 2: " << e.face2;
    os << ", Message: " << e.message;
    return os;
  }

  bool inspect_face(const std::vector<pmp::Point>& coords)
  {
    // boost polygon representation
    boost_polygon_t poly = make_polygon(coords);
    
    // the face should be valid & simple
    return (bg::is_valid(poly) && bg::is_simple(poly));
  }
}
