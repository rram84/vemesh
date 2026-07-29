// Sriramajayam

#include <vm_TetMesh.h>
#include <cassert>
#include <map>

// boost polygon utilities
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace vm
{
  // boost aliases
  namespace bg  = boost::geometry;
  namespace bgm = bg::model;
  using boost_point_t    = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_polygon_t  = bgm::polygon<boost_point_t, false>;
  using boost_linestring_t = bgm::linestring<boost_point_t>;

  // checks whether the area of a 2D triangle is positive
  bool is_triangle_oriented(const double* A, const double* B, const double* C)
  {
    boost_polygon_t poly{{{A[0],A[1]}, {B[0],B[1]}, {C[0],C[1]}, {A[0],A[1]}}};
    return (bg::area(poly)>0.);
  }

  // checks whether 4 points form a quadrilateral
  bool is_quadrilateral_simple(const double* A, const double* B, const double* C, const double* D)
  {
    boost_linestring_t ls{{A[0],A[1]}, {B[0],B[1]}, {C[0],C[1]}, {D[0],D[1]}, {A[0],A[1]}};
    return bg::is_simple(ls);
  }


  // checks whether the area of a 2D quadrilateral is positive
  bool is_quadrilateral_oriented(const double* A, const double* B, const double* C, const double* D)
  {
    boost_polygon_t poly{{{A[0],A[1]}, {B[0],B[1]}, {C[0],C[1]}, {D[0],D[1]}, {A[0],A[1]}}};
    return (bg::area(poly)>0.);
  }
  
  // slice a tet mesh at a z-plane
  pmp::SurfaceMesh TetMesh::zslice(const double zcoord) const 
  {
    assert(num_nodes>0 && num_elements>0);

    // local enumerations
    const int local_edges[]     = {0,1, 0,2, 0,3, 1,2, 1,3, 2,3};
    const int num_edges_per_elm = 6;
    const int num_quad_permutations   = 3;
    const int quad_permutations[3][4] = {{0,1,2,3},
					 {0,1,3,2},
					 {0,2,1,3}};
    
    // enumerate all edges that are intersected by the given plane
    using Edge_t = std::pair<int,int>;
    std::vector<std::array<double,2>> cut_coords{};
    std::vector<std::vector<int>>     cut_conn{};
    std::map<Edge_t, int>             cut_edge_index{};
    
    for(auto& tet_conn:connectivity)
      {
	// indices of intersection points
	std::vector<int> my_cut_indices{};
	
	for(int e=0; e<num_edges_per_elm; ++e)
	  {
	    int a = tet_conn[local_edges[2*e]];
	    int b = tet_conn[local_edges[2*e+1]];
	    if(a>b)
	      std::swap(a,b);
	    
	    const auto& A = coordinates[a];
	    const auto& B = coordinates[b];
	    if((A[2]-zcoord)*(B[2]-zcoord)<0.)
	      {
		auto it = cut_edge_index.find({a,b});

		// new intersection point?
		if(it==cut_edge_index.end())
		  {
		    // this is a new edge cut by the plane

		    // new index
		    const int new_index = static_cast<int>(cut_coords.size());
		    cut_edge_index.insert({{a,b},new_index});
		    my_cut_indices.push_back(new_index);

		    // new coordinates
		    const double lambda = (zcoord-A[2])/(B[2]-A[2]);
		    cut_coords.push_back({A[0]+lambda*(B[0]-A[0]), A[1]+lambda*(B[1]-A[1])});
		  }
		else
		  {
		    // existing intersection point
		    my_cut_indices.push_back(it->second);
		  }
	      }
	  } // end loop over edges of a tet

	// examine the intersection in this element
	if(!my_cut_indices.empty())
	  {
	    const int num_intersections = static_cast<int>(my_cut_indices.size());
	    assert(num_intersections==3 || num_intersections==4);

	    // in case of a triangle intersection, only the orientation needs to be set
	    if(num_intersections==3)
	      {
		std::array<const double*,3> tri_coords;
		for(int a=0; a<3; ++a)
		  tri_coords[a] = cut_coords[my_cut_indices[a]].data();

		if(is_triangle_oriented(tri_coords[0], tri_coords[1], tri_coords[2])==true)
		  cut_conn.push_back({my_cut_indices[0], my_cut_indices[1], my_cut_indices[2]});
		else
		  cut_conn.push_back({my_cut_indices[0], my_cut_indices[2], my_cut_indices[1]});
	      }
	    // for a qudrilateral interserction, the ordering of points and the orientation need to be set
	    else if(num_intersections==4)
	      {
		// the four points
		std::array<const double*,4> quad_coords;
		for(int a=0; a<4; ++a)
		  quad_coords[a] = cut_coords[my_cut_indices[a]].data();

		// examine all 3 possible permutations of the intersection points
		bool success = false;
		for(int p=0; p<num_quad_permutations; ++p)
		  {
		    const int* perm = quad_permutations[p];
		    if(is_quadrilateral_simple(quad_coords[perm[0]], quad_coords[perm[1]], quad_coords[perm[2]], quad_coords[perm[3]])==true)
		      {
			if(is_quadrilateral_oriented(quad_coords[perm[0]], quad_coords[perm[1]], quad_coords[perm[2]], quad_coords[perm[3]])==true)
			  cut_conn.push_back({my_cut_indices[perm[0]], my_cut_indices[perm[1]], my_cut_indices[perm[2]], my_cut_indices[perm[3]]});
			else
			  cut_conn.push_back({my_cut_indices[perm[0]], my_cut_indices[perm[3]], my_cut_indices[perm[2]], my_cut_indices[perm[1]]});

			success = true;
			break;
		      }
		  }
		
		// one permutation should work
		assert(success==true);
	      }

	  } // end loop over non-empty intersections
	
      } // end loop over tets
    
    
    // sanity checks
    assert(cut_coords.size()>0 && cut_conn.size()>0);
    const int num_vertices = static_cast<int>(cut_coords.size());
    std::set<int> vert_set{};
    for(auto& conn:cut_conn)
      for(auto& n:conn)
	vert_set.insert(n);
    assert(static_cast<int>(vert_set.size())==num_vertices);
    int count = 0;
    for(auto& n:vert_set)
      assert(n==count++);
    
    // convert to a surface mesh
    pmp::SurfaceMesh surface_mesh;
    std::vector<pmp::Vertex> vertices{};
    for(auto& X:cut_coords)
      vertices.push_back(surface_mesh.add_vertex(pmp::Point(X[0],X[1],zcoord)));
    for(auto& conn:cut_conn)
      {
	std::vector<pmp::Vertex> face_verts{};
	for(auto& n:conn)
	  face_verts.push_back(vertices[n]);
	surface_mesh.add_face(face_verts);
      }

    assert(surface_mesh.n_vertices()==static_cast<int>(cut_coords.size()));
    assert(surface_mesh.n_faces()==static_cast<int>(cut_conn.size()));
    
    // done
    return surface_mesh;
  }
  
}
