// Sriramajayam

#include <vm_Manager.h>
#include <vm_vertex_ring.h>
#include <boost/geometry/geometry.hpp>
#include <random>
#include <queue>
#include <iostream>

namespace vm
{
  namespace bg             = boost::geometry;
  namespace bgm            = boost::geometry::model;
  using boost_point_t      = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_polygon_t    = bgm::polygon<boost_point_t, false>;
  using boost_box_t        = bgm::box<boost_point_t>;
  using boost_linestring_t = bgm::linestring<boost_point_t>;
  
  // moves a vertex
  std::pair<bool, double> Manager::move_vertex(const pmp::Vertex& vertex, const int num_poly_samples, const int num_edge_samples, MeshVertexQuality_f qfunc)
  {
    // cannot move boundary vertices
    if(mesh.is_boundary(vertex)==true)
      return {false, -1.0};
    
    // identify a feasible new position & move
    const auto result = compute_improved_vertex_position(mesh, vertex, num_poly_samples, num_edge_samples, qfunc);
        
    // no feasible point
    if(std::get<0>(result)==false)
      return {false, -1.0};
    
    // found a feasible point
    const auto& update_pos = std::get<1>(result);
    
    // move
    pmp::Point& X = mesh.position(vertex);
    X = update_pos;
    
    // done
    return {true, std::get<2>(result)};
  }


  // alias
  using VQ_pair_t = std::pair<pmp::Vertex, double>;
  
  // Custom comparator of vertex/quality pairs
  bool Compare(const VQ_pair_t& A, const VQ_pair_t& B)
  { return A.second>B.second; }
  
  // moves vertices
  int Manager::move_vertices(MeshVertexQuality_f qfunc, const double qmin,
			     const int num_poly_samples, const int num_edge_samples,
			     MeshUpdateCallback_f callback)
  {
    // tolerance for comparing qualities
    const double qeps = qmin/100.;
    
    // priority queue of vertices to be relaxed during this iteration
    std::priority_queue<VQ_pair_t, std::vector<VQ_pair_t>, decltype(&Compare)> vertex_queue(Compare);
    auto v_container = mesh.vertices();
    for(auto v:v_container)
      if(mesh.is_boundary(v)==false)
	{
	  double qval = qfunc(mesh, v);
	  if(qval<qmin)
	    vertex_queue.push({v, qval});
	}
    const int qsize = static_cast<int>(vertex_queue.size());
    std::cout << "#vertices marked for relaxation: " << qsize << std::endl;

    // #vertices relaxed during this iteration
    int nrelaxed = 0;
    int prev_percent = 0;
    
    // traverse the queue
    while(!vertex_queue.empty())
      {
	int percent_complete = (static_cast<int>(vertex_queue.size())*100)/qsize;
	if(percent_complete>prev_percent+20)
	  {
	    std::cout << "Progress: " << prev_percent+20 << "%" << std::endl;
	    prev_percent += 20;
	  }
	
	// pop the first vertex in the queue
	auto vq = vertex_queue.top();
	const auto& v = vq.first;
	vertex_queue.pop();

	// do nothing if:
	// the quality at this vertex, which could have changed due to other vertices
	// moving, is > qmin
	const double curr_q = qfunc(mesh, v);
	if(curr_q>qmin)
	  continue;
	
	// reposition this vertex in the queue if its quality has changed
	if(std::abs(curr_q-vq.second)>qeps)
	  {
	    vertex_queue.push({vq.first,curr_q});
	    continue;
	  }

	// this vertex is the current priority
	auto result = this->move_vertex(v, num_poly_samples, num_edge_samples, qfunc);
	auto success = result.first;
	if(success==true)
	  {
	    ++nrelaxed;
	    if(callback!=nullptr)
	      callback(nrelaxed, mesh, *this);
	  }
      }
    std::cout << "Progress: 100%" << std::endl;
    std::cout << "#vertices relaxed: " << nrelaxed << std::endl;
    return nrelaxed;
  }


  // identify a feasible point to move a vertex
  std::tuple<bool, pmp::Point, double> Manager::compute_improved_vertex_position(pmp::SurfaceMesh          &mesh,
										 const pmp::Vertex         &vertex,
										 const int                 num_poly_samples,
										 const int                 num_edge_samples,
										 const MeshVertexQuality_f qfunc)
  {
    assert(mesh.is_valid(vertex)==true);
    assert(mesh.is_boundary(vertex)==false);

    // given vertex position
    const pmp::Point given_vertex_pos = mesh.position(vertex);

    // get feasible sample points inside the visibility polygon
    const std::vector<std::pair<double,double>> feasible_samples = compute_feasible_vertex_positions(mesh, vertex, num_poly_samples, num_edge_samples);
    
    // use the current vertex quality as the datum
    std::pair<double,double> curr_best_pos = {given_vertex_pos[0], given_vertex_pos[1]};
    double curr_best_quality = qfunc(mesh, vertex);
    
    // examine vertex qualities at the sample points
    pmp::Point& running_vert_pos = mesh.position(vertex);
    bool success = false;
    for(auto& sample:feasible_samples)
      {
	// move the vertex to this sample point's location
	running_vert_pos[0] = sample.first;
	running_vert_pos[1] = sample.second;
	
	// evaluate the resulting vertex quality
	double sample_quality = qfunc(mesh, vertex);
	
	// Does sample_quality dominate curr_best_quality
	if(sample_quality>curr_best_quality)
	  {
	    curr_best_quality = sample_quality;
	    curr_best_pos               = sample;
	    success                     = true;
	  }
      }
    
    // restore the vertex position
    mesh.position(vertex) = given_vertex_pos;

    // done
    return {success, pmp::Point(curr_best_pos.first,curr_best_pos.second,given_vertex_pos[2]), curr_best_quality};
  }


  // Query whether a point is feasible
  bool feasibility_test_1(const boost_polygon_t         &poly,
			  const std::vector<pmp::Point> &connectedVertices,
			  const boost_point_t           &sample)
  {
    // does this point lie within the polygon
    if(bg::within(sample, poly)==false)
      return false;
    
    // do the segments joining this point to the connected vertices lie within the polygon?
    const double EPS = 0.01;
    for(auto& Y:connectedVertices)
      {
	boost_linestring_t seg;
	bg::append(seg, sample);
	bg::append(seg, boost_point_t(EPS*bg::get<0>(sample)+(1.-EPS)*Y[0],
				      EPS*bg::get<1>(sample)+(1.-EPS)*Y[1])); // boundary vertex moved inward
	if(bg::within(seg, poly)==false)
	  return false;
      }

    // done
    return true;
  }


  // Query if a point is feasible
  // check that all faces incident at a vertex are simple when its position is perturbed
  bool feasibility_test_2(const pmp::SurfaceMesh &mesh,
			  const pmp::Vertex      &vertex,
			  const boost_point_t    &sample)
  {
    // faces incident at v
    auto face_circulator = mesh.faces(vertex);

    for(auto f:face_circulator)
      {
	// this perturbed face
	boost_polygon_t poly;
	auto vert_circulator = mesh.vertices(f);
	for(auto w:vert_circulator)
	  {
	    if(w.idx()==vertex.idx())
	      bg::append(poly.outer(), sample);
	    else
	      {
		const auto& X = mesh.position(w);
		bg::append(poly.outer(), boost_point_t(X[0], X[1]));
	      }
	  }

	// close te polygon
	auto first_vertex = *poly.outer().begin();  
	bg::append(poly.outer(), first_vertex);
	
	// is this polygon valid
	if(!bg::is_valid(poly))
	  return false;
	
	// is this polygon simple
	if(!bg::is_simple(poly))
	  return false;
      }
    return true;
  }
    
  
  // random generation of feasible vertex positions
  std::vector<std::pair<double,double>>
  Manager::compute_feasible_vertex_positions(const pmp::SurfaceMesh &mesh,
					     const pmp::Vertex      &vertex,
					     const int              num_poly_samples,        // max number of random positions to generate
					     const int              num_edge_samples)        // number of samples to generate per edge  
  {
    // boost polygon of the environment around the vertex
    boost_polygon_t poly;
    auto vertex_ring = get_vertex_ring(mesh, vertex);
    for(auto& v:vertex_ring)
      {
	const auto& X = mesh.position(v);
	bg::append(poly.outer(), boost_point_t(X[0], X[1]));
      }
    bg::correct(poly);

    // axis-aligned bounding box for poly
    boost_box_t bbox;
    bg::envelope(poly, bbox);
    const auto& min_corner = bbox.min_corner();
    const auto& max_corner = bbox.max_corner();

    // outgoing halfedges from vertex
    auto out_halfedges = mesh.halfedges(vertex);

    // neighbors to which "vertex" is connected
    std::vector<pmp::Point> connected_vertices{};
    for(auto h:out_halfedges)
      {
	assert(mesh.from_vertex(h)==vertex);
	connected_vertices.push_back(mesh.position(mesh.to_vertex(h)));
      }
        
    // Random generator
    std::random_device rd; 
    std::mt19937 gen(rd());

    // all samples
    std::vector<boost_point_t> samples{};

    // sample the bounding box
    std::uniform_real_distribution<> xdis(bg::get<0>(min_corner), bg::get<0>(max_corner));
    std::uniform_real_distribution<> ydis(bg::get<1>(min_corner), bg::get<1>(max_corner));
    for(int iter=0; iter<num_poly_samples; ++iter)
      samples.push_back(boost_point_t(xdis(gen), ydis(gen)));

    // convex combinations of connected vertices
    std::uniform_real_distribution<> lambda_dis(0.,1.);
    const int nconn = static_cast<int>(connected_vertices.size());
    std::vector<double> wts(nconn);
    double wsum;
    double pt[2];
    for(int iter=0; iter<num_edge_samples; ++iter) {

      // weights
      for(int i=0; i<nconn; ++i) {
	wts[i] = lambda_dis(gen);
	wsum += wts[i];
      }

      // normalize
      for(int i=0; i<nconn; ++i)
	wts[i] /= wsum;

      // sample point
      pt[0] = pt[1] = 0.;
      for(int i=0; i<nconn; ++i) {
	pt[0] += wts[i]*connected_vertices[i][0];
	pt[1] += wts[i]*connected_vertices[i][1];
      }
      samples.push_back(boost_point_t(pt[0], pt[1]));
    }
      
    // sample incident edges
    // const pmp::Point& Xv = mesh.position(vertex);
    //   for(auto& Y:connected_vertices)
    //   for(int iter=0; iter<num_edge_samples; ++iter)
    //   {
    //   const double lambda = lambda_dis(gen);
    //   samples.push_back(boost_point_t(lambda*Xv[0]+(1.-lambda)*Y[0],
    //   lambda*Xv[1]+(1.-lambda)*Y[1]));
    //   }

    // output = subset of feasible samples
    std::vector<std::pair<double,double>> feasible_points{};
    for(auto& sample:samples)
      if(feasibility_test_1(poly, connected_vertices, sample)==true &&
	 feasibility_test_2(mesh, vertex, sample)==true) {
	feasible_points.push_back({bg::get<0>(sample), bg::get<1>(sample)});
      }

    // done
    return std::move(feasible_points);
  }

}
