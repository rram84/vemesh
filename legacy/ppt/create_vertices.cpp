// Sriramajayam

#include <fstream>
#include <iostream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace bg = boost::geometry;
typedef boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian> boost_point2D;
typedef std::pair<boost_point2D, int> pointID;
typedef bg::model::box<boost_point2D> boost_box;


int main()
{
  boost::geometry::index::rtree<pointID, boost::geometry::index::quadratic<8>> rtree, ptree;
  rtree.clear();
  
  std::fstream pfile;
  pfile.open("raw.dat", std::ios::in);
  double pt[2];
  pfile >> pt[0];
  int indx = 0;
  while(pfile.good())
    {
      pfile >> pt[1];
      rtree.insert({boost_point2D(pt[0], pt[1]),indx++});
      pfile >> pt[0];
    }
  pfile.close();
  
  std::cout << "Read " << rtree.size() << " points " << std::endl;

  // Erase points that are within a given bounding box around each point in the tree
  const double h = 5.e-3;
  while(!rtree.empty())
    {
      auto it = rtree.begin();
      // create a bounding box around this point
      const double C[] = {bg::get<0>(it->first), bg::get<1>(it->first)};
      ptree.insert(*it);
      boost_box bbox(boost_point2D(C[0]-h, C[1]-h), boost_point2D(C[0]+h, C[1]+h));
      std::vector<pointID> result{};
      rtree.query(bg::index::within(bbox), std::back_inserter(result));
      for(auto& jt:result)
	rtree.remove(jt);
    }

  pfile.open("ptree.dat", std::ios::out);
  for(auto& it:ptree)
    pfile << bg::get<0>(it.first) << " " << bg::get<1>(it.first) << std::endl;
  pfile.close();
  
  // vertices in sequence
  std::vector<boost_point2D> seq_verts{};

  // seed point
  for(auto& it:ptree)
    {
      seq_verts.push_back(it.first);
      ptree.remove(it);
      break;
    }

  while(!ptree.empty())
    {
      // last point in the sequence
      auto& X = seq_verts.back();

      // closest point in the tree
      std::vector<pointID> result{};
      ptree.query(bg::index::nearest(X,1), std::back_inserter(result));
      assert(result.size()==1);
      seq_verts.push_back(result[0].first);
      ptree.remove(result[0]);
    }
  const int nverts = static_cast<int>(seq_verts.size());
  std::cout << "Number of vertices: " << nverts << std::endl;
  pfile.open("vertices.dat", std::ios::out);
  indx = 0;
  for(auto& p:seq_verts)
    pfile << indx++ << " " << bg::get<0>(p) << " " << bg::get<1>(p) << std::endl;
  pfile.close();
}
