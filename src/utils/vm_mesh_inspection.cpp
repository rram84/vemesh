// Sriramajayam

/** \file vm_mesh_inspection.cpp
 * \brief Implements utilities for mesh inspection
 * \author Ramsharan Rangarajan
 */

#include <vm_mesh_inspection.h>
#include <sstream>

namespace vm
{
  namespace {
    
    // Helper to add error if optional list is provided
    void add_error(const std::optional<std::reference_wrapper<MeshInspectionErrors>> &errors, MeshInspectionError e) {
      if (errors) errors->get().push_back(std::move(e));
    }

  

    // ---- basic mesh inspection tests. only sequential. --- //
    bool inspect_mesh_basic(const pmp::SurfaceMesh& mesh,
			    const std::optional<std::reference_wrapper<MeshInspectionErrors>> &errors)
    {
      bool flag = true;
      if(mesh.n_vertices()==0 || mesh.n_faces()==0 || mesh.n_edges()==0)
	{
	  flag = false;
	  add_error(errors, {MeshInspectionErrorCode::EmptyMesh,-1, -1, "Empty mesh"});
	}
	
      return flag;
    }


    // Merge thread-local findings into the optional shared error list.
    // Should be called within a critical section
    void merge_errors(const std::optional<std::reference_wrapper<MeshInspectionErrors>>& errors,
		      MeshInspectionErrors& local)
    {
      auto& dst = errors->get();
      dst.insert(dst.end(),
		 std::make_move_iterator(local.begin()),
		 std::make_move_iterator(local.end()));
    }


    // Thread-safe geometry checks for a single face. Appends any findings to `out`.
    // Returns true if the face is valid, simple and positively oriented.
    bool check_face(int findx, const boost_polygon_t& poly, MeshInspectionErrors& out)
    {
      bool ok = true;

      std::string message;
      if(!bg::is_valid(poly, message))
	{
	  ok = false;
	  std::ostringstream oss;
	  oss << "Boost message: " << message << "\nFace coordinates: " << boost::geometry::wkt(poly) << "\n";
	  out.push_back({MeshInspectionErrorCode::InvalidFace, findx, -1, oss.str()});
	}
      if(!bg::is_simple(poly))
	{
	  ok = false;
	  std::ostringstream oss;
	  oss << "Face coordinates: " << boost::geometry::wkt(poly) <<"\n";
	  out.push_back({MeshInspectionErrorCode::NonSimpleFace, findx, -1, oss.str()});
	}
      if(bg::area(poly)<=0.)
	{
	  ok = false;
	  std::ostringstream oss;
	  oss <<"Face coordinates: " << boost::geometry::wkt(poly) << "\n";
	  out.push_back({MeshInspectionErrorCode::NonPositiveArea, findx, -1, oss.str()});
	}

      return ok;
    }

    // Thread-safe checks for overlap checks for a single face against its neighbors.
    // Appends findings to `out`.
    // Each adjacent pair is examined once (findx < nb_findx).
    // Returns true if the face does not overlap any neighbor.
    bool check_overlaps(const pmp::SurfaceMesh& mesh,
			const pmp::Face& face,
			const std::map<int, boost_polygon_t>& polygons,
			MeshInspectionErrors& out)
    {
      const int    findx = face.idx();
      const auto&  poly  = polygons.at(findx);
      const double area  = bg::area(poly);
      bool ok = true;

      // its neighbors
      auto halfedge_circulator = mesh.halfedges(face);
      for(auto h:halfedge_circulator)
	{
	  // skip when there is no neighboring face across this halfedge
	  auto h_opp = mesh.opposite_halfedge(h);
	  if(mesh.is_boundary(h_opp))
	    continue;
	  
	  // neighboring face; avoid double checking pairwise insersections
	  auto nb_face = mesh.face(h_opp);
	  const int nb_findx = nb_face.idx();
	  if(findx>nb_findx)
	    continue;

	  // check pairwise intersection
	  const auto& nb_poly = polygons.at(nb_findx);
	  boost_multi_polygon_t intersection;
	  bg::intersection(poly, nb_poly, intersection);
	  double intersection_area = bg::area(intersection);

	  if(std::abs(intersection_area/area)>1.e-3) {
	    ok = false;
	    std::ostringstream oss;
	    oss << "Face: " << boost::geometry::wkt(poly) << "\nNeighbor face: " << boost::geometry::wkt(nb_poly) << "\n";
	    out.push_back({MeshInspectionErrorCode::FaceOverlap, findx, nb_findx, oss.str()});
	  }
	}

      return ok;
    }


    // enum to string conversion for error messages
    std::string to_string(const MeshInspectionErrorCode code)
    {
      switch (code) {
      case MeshInspectionErrorCode::EmptyMesh:       return "EmptyMesh";
      case MeshInspectionErrorCode::InvalidFace:     return "InvalidFace";
      case MeshInspectionErrorCode::NonSimpleFace:   return "NonSimpleFace";
      case MeshInspectionErrorCode::NonPositiveArea: return "NonPositiveArea";
      case MeshInspectionErrorCode::FaceOverlap:     return "FaceOverlap";
      default:                                   return "UnknownError";
      }
    }

    
    // --- inspect mesh faces, parallelized if OpenMP is available --- //
    bool inspect_mesh_faces(const std::map<int, boost_polygon_t>& polygons,
			    const std::optional<std::reference_wrapper<MeshInspectionErrors>> &errors)
    {
      // flat index list of polygons so that per-face checks can be iterated by index (OpenMP)
      std::vector<int> keys;
      keys.reserve(polygons.size());
      for(const auto& kv : polygons)
	keys.push_back(kv.first);

      const int n = static_cast<int>(keys.size());
      bool flag = true;

      // each face should be valid, simple and have a positive area
#pragma omp parallel for schedule(dynamic) reduction(&&:flag)
      for(int i=0; i<n; ++i)
	{
	  MeshInspectionErrors local;
	  const bool ok = check_face(keys[i], polygons.at(keys[i]), local);
	  flag = flag && ok;
	  if(errors && !local.empty())
	    {
#pragma omp critical
	      merge_errors(errors, local);
	    }
	}

      return flag;
    }



    // --- inspect overlaps ---    
    bool inspect_mesh_adjacency(const pmp::SurfaceMesh& mesh,
				const std::map<int, boost_polygon_t>& polygons,
				const std::optional<std::reference_wrapper<MeshInspectionErrors>> &errors)
    {
      // flat index list for index-based (OpenMP) iteration
      std::vector<int> keys;
      keys.reserve(polygons.size());
      for(const auto& kv : polygons)
	keys.push_back(kv.first);

      const int n = static_cast<int>(keys.size());
      bool flag = true;

      // faces should not overlap with neighbors
      // check intersection with neighbors
#pragma omp parallel for schedule(dynamic) reduction(&&:flag)
      for(int i=0; i<n; ++i)
	{
	  // this face
	  const pmp::Face face(static_cast<pmp::IndexType>(keys[i]));

	  // inspect this face for overlap with neighbors
	  MeshInspectionErrors local;
	  const bool ok = check_overlaps(mesh, face, polygons, local);
	  flag = flag && ok;

	  if(errors && !local.empty())
	    {
#pragma omp critical
	      merge_errors(errors, local);
	    }
	}

      return flag;
    }

  }
  
    
  // ---- mesh-level inspection -- //
  bool inspect_mesh(const pmp::SurfaceMesh& mesh,
		    MeshInspection level,
		    std::optional<std::reference_wrapper<MeshInspectionErrors>> errors)
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


  // print error messages
  std::ostream& operator<<(std::ostream& os, const MeshInspectionError& e)
  {
    os << "Code: " << to_string(e.code)
       << ", Face: " << e.face;
    if (e.face2 != -1)
      os << ", Face 2: " << e.face2;
    os << ", Message: " << e.message;
    return os;
  }

  // inspect a face
  bool inspect_face(const std::vector<pmp::Point>& coords)
  {
    // boost polygon representation
    boost_polygon_t poly = make_polygon(coords);
    
    // the face should be valid & simple
    return (bg::is_valid(poly) && bg::is_simple(poly));
  }
}
