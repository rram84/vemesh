#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Simple_polygon_visibility_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_naive_point_location.h>
#include <istream>
#include <vector>
typedef CGAL::Exact_predicates_exact_constructions_kernel             Kernel;
typedef Kernel::Point_2                                                 Point_2;
typedef Kernel::Segment_2                                               Segment_2;
typedef CGAL::Arr_segment_traits_2<Kernel>                              Traits_2;
typedef CGAL::Arrangement_2<Traits_2>                                   Arrangement_2;
typedef Arrangement_2::Face_handle                                      Face_handle;
typedef Arrangement_2::Edge_const_iterator                              Edge_const_iterator;
typedef Arrangement_2::Halfedge_const_iterator                          Halfedge_const_iterator;
typedef Arrangement_2::Ccb_halfedge_circulator                          Ccb_halfedge_circulator;
using Vertex_const_iterator   = Arrangement_2::Vertex_const_iterator;

int main() {
  //create environment
  Point_2
    p1(70, 76),
    p2(70, 74),
    p3(72, 74),
    p4(72.9206, 71.733),
    p5(73.9048, 74.0952),
    p6(73.9048, 76),
    p7(73.5246, 76.4162),
    p8(73.5724, 77.1908),
    p9(72.7337, 77.8255),
    p10(71.985, 76.9184),
    p11(70.0952, 77.9048);

  std::vector<Segment_2> segments;
  segments.push_back(Segment_2(p1, p2));
  segments.push_back(Segment_2(p2, p3));
  segments.push_back(Segment_2(p3, p4));
  segments.push_back(Segment_2(p4, p5));
  segments.push_back(Segment_2(p5, p6));
  segments.push_back(Segment_2(p6, p7));
  segments.push_back(Segment_2(p7, p8));
  segments.push_back(Segment_2(p8, p9));
    segments.push_back(Segment_2(p9, p10));
    segments.push_back(Segment_2(p10, p11));
    segments.push_back(Segment_2(p11, p1));
  std::cout << "Number of segments: "<< segments.size() << std::endl;
  Arrangement_2 env;
  CGAL::insert_non_intersecting_curves(env,segments.begin(),segments.end());
  // find the face of the query point
  // (usually you may know that by other means)
  Point_2 q(71.9852, 76.9092);
  Arrangement_2::Face_const_handle * face;
  CGAL::Arr_naive_point_location<Arrangement_2> pl(env);
  CGAL::Arr_point_location_result<Arrangement_2>::Type obj = pl.locate(q);
  // The query point locates in the interior of a face
  face = boost::get<Arrangement_2::Face_const_handle> (&obj);
  // compute non regularized visibility area
  // Define visibiliy object type that computes non-regularized visibility area
  typedef CGAL::Simple_polygon_visibility_2<Arrangement_2, CGAL::Tag_false> NSPV;
  Arrangement_2 non_regular_output;
  NSPV non_regular_visibility(env);
  non_regular_visibility.compute_visibility(q, *face, non_regular_output);
  std::cout << "Non-regularized visibility region of q has "
            << non_regular_output.number_of_edges()
            << " edges:" << std::endl;
  for (Edge_const_iterator eit = non_regular_output.edges_begin(); eit != non_regular_output.edges_end(); ++eit)
    std::cout << "[" << eit->source()->point() << " -> " << eit->target()->point() << "]" << std::endl;
  // compute non regularized visibility area
  // Define visibiliy object type that computes regularized visibility area
  typedef CGAL::Simple_polygon_visibility_2<Arrangement_2, CGAL::Tag_true> RSPV;
  Arrangement_2 regular_output;
  RSPV regular_visibility(env);
  regular_visibility.compute_visibility(q, *face, regular_output);
  std::cout << "Regularized visibility region of q has "
            << regular_output.number_of_edges()
            << " edges:" << std::endl;
  for (Edge_const_iterator eit = regular_output.edges_begin(); eit != regular_output.edges_end(); ++eit)
    std::cout << eit->source()->point() << std::endl << eit->target()->point() << std::endl << std::endl;

  
  std::cout << std::endl << std::endl << "Vertices: " << std::endl;
  auto v_begin = regular_output.vertices_begin();
  auto v_end   = regular_output.vertices_end();
  for(Vertex_const_iterator it=v_begin; it!=v_end; ++it)
    {
      const auto& P = it->point();
      std::cout << P.x() << " " << P.y() << std::endl;
    }

  std::cout << std::endl << std::endl << "Printing halfedges " << std::endl << std::endl;
  auto h_begin = regular_output.halfedges_begin();
  auto h = h_begin;
  const int num_hedges = regular_output.number_of_halfedges()/2;
  for(int count=0; count<num_hedges; ++count)
    {
      std::cout << h->source()->point() <<  std::endl << h->target()->point() << std::endl << std::endl;
      h = h->next();
    }

  /*
  auto h_end   = regular_output.halfedges_end();
  for(auto h=h_begin; h!=h_end; ++h)
    std::cout << h->source()->point() <<  std::endl << h->target()->point() << std::endl << std::endl;
  */
  
  //Ccb_halfedge_circulator
}
