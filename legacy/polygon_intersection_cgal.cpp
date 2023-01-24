// Sriramajayam

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <list>
typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef Kernel::Point_2                                   Point_2;
typedef CGAL::Polygon_2<Kernel>                           Polygon_2;
typedef CGAL::Polygon_with_holes_2<Kernel>                Polygon_with_holes_2;
typedef std::list<Polygon_with_holes_2>                   Pwh_list_2;


template<class MyKernel, class Container>
void print_polygon (const CGAL::Polygon_2<MyKernel, Container>& P)
{
  typename CGAL::Polygon_2<MyKernel, Container>::Vertex_const_iterator vit;
  std::cout << "[ " << P.size() << " vertices:";
  for (vit = P.vertices_begin(); vit != P.vertices_end(); ++vit)
    std::cout << *vit << std::endl;
  std::cout << " ]" << std::endl;
}

template<class MyKernel, class Container>
void print_polygon_with_holes(const CGAL::Polygon_with_holes_2<MyKernel, Container> & pwh)
{
  if (! pwh.is_unbounded()) {
    std::cout << "{ Outer boundary = ";
    print_polygon (pwh.outer_boundary());
  } else
    std::cout << "{ Unbounded polygon." << std::endl;
  typename CGAL::Polygon_with_holes_2<MyKernel,Container>::Hole_const_iterator hit;
  unsigned int k = 1;
  std::cout << " " << pwh.number_of_holes() << " holes:" << std::endl;
  for (hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit, ++k) {
    std::cout << " Hole #" << k << " = ";
    print_polygon (*hit);
  }
  std::cout << " }" << std::endl;
}


int main()
{
  // Construct the two input polygons.
  Polygon_2 P;
  P.push_back(Point_2(76.37593982020243, 53.62855965544195));
  P.push_back(Point_2(76.56629943847656, 53.71099853515625));
  P.push_back(Point_2(76.97709655761719, 54));
  P.push_back(Point_2(77.41490173339844, 54.58509826660156));
  P.push_back(Point_2(77, 55));
  P.push_back(Point_2(76.99999312109907, 55.00000687887207));
  P.push_back(Point_2(76.99866406552184, 55.00133592887243));
  P.push_back(Point_2(76.09896357657894, 55.90103264257367));
  P.push_back(Point_2(76.32990264892578, 54));
  //P.push_back(Point_2(76.37593982020243, 53.62855965544195));


  Polygon_2 Q;
  Q.push_back(Point_2(77.41490173339844, 54.58509826660156));
  Q.push_back(Point_2(77, 55));
  Q.push_back(Point_2(76.09089660644531, 55.90909957885742));
  Q.push_back(Point_2(76, 56));
  Q.push_back(Point_2(76.32990264892578, 54));
  Q.push_back(Point_2(76.372802734375, 53.62720108032227));
  Q.push_back(Point_2(76.56629943847656, 53.71099853515625));
  Q.push_back(Point_2(76.97709655761719, 54));
  //Q.push_back(Point_2(77.41490173339844, 54.58509826660156));
  
  
  Pwh_list_2                  intR;
  Pwh_list_2::const_iterator  it;
  CGAL::intersection (P, Q, std::back_inserter(intR));

  std::cout << "Intersection size: " << intR.size() << std::endl;
  for (it = intR.begin(); it != intR.end(); ++it)
    {
      print_polygon_with_holes (*it);
    }
}
