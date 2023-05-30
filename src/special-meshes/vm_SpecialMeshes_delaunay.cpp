// Sriramajayam

#include <vm_SpecialMeshes.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <random>

namespace vm
{
  // CGAL aliases
  using Kernel   = CGAL::Exact_predicates_inexact_constructions_kernel;
  using Vb       = CGAL::Triangulation_vertex_base_with_info_2<unsigned int, Kernel>;
  using Tds      = CGAL::Triangulation_data_structure_2<Vb>;
  using Delaunay = CGAL::Delaunay_triangulation_2<Kernel, Tds>;
  using Point    = Kernel::Point_2;
  
  // compute delaunay triangulation
  pmp::SurfaceMesh create_delaunay_triangulation(const std::vector<std::pair<double,double>>& points)
  {
    const int nPoints = static_cast<int>(points.size());
    assert(nPoints>=3);
    std::vector<std::pair<Point,unsigned int>> cgal_points(nPoints);
    for(int p=0; p<nPoints; ++p)
      cgal_points[p] = {Point(points[p].first, points[p].second) ,p};
    
    Delaunay dt;
    dt.insert(cgal_points.begin(), cgal_points.end());
    const int nvertices = dt.number_of_vertices();
    const int nfaces = dt.number_of_faces();
    assert(nvertices>=3 && nfaces>=1);
    
    // export to SurfaceMesh
    pmp::SurfaceMesh mesh;

    // vertices
    auto vbegin = dt.finite_vertices_begin();
    auto vend   = dt.finite_vertices_end();
    std::vector<pmp::Vertex> vertices(nvertices);
    int vcount = 0;
    for(auto v=vbegin; v!=vend; ++v)
      {
	++vcount;
	const int indx  = v->info(); 
	const auto& pt  = v->point();
	vertices[indx] = mesh.add_vertex(pmp::Point(pt.x(), pt.y(), 0.0));
      }
    assert(vcount==nvertices);
    
    // triangles
    auto fbegin = dt.finite_faces_begin();
    auto fend   = dt.finite_faces_end();
    for(auto f=fbegin; f!=fend; ++f)
      {
	Delaunay::Face_handle face = f;
	const unsigned int idx[] = {face->vertex(0)->info(), face->vertex(1)->info(), face->vertex(2)->info()};
	mesh.add_face({vertices[idx[0]], vertices[idx[1]], vertices[idx[2]]});
      }
        
    return mesh;
  }



  // delaunay triangulation of a random distribution of points within the unit square
  pmp::SurfaceMesh create_random_delaunay(const double* bot_left_cnr,
					  const double* top_right_cnr,
					  const int num_points)
  {
    // generate a random collection of points
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_real_distribution<> xdis(bot_left_cnr[0], top_right_cnr[0]);
    std::uniform_real_distribution<> ydis(bot_left_cnr[1], top_right_cnr[1]);
    std::vector<std::pair<double,double>> points(num_points);
    for(int i=0; i<num_points; ++i)
      points[i] = {xdis(gen), ydis(gen)};

    return create_delaunay_triangulation(points);
  }
  
}
