// Sriramajayam

/** \file vm_mesh_optimizer_relax.cpp
 * \brief Implementation of vertex relaxation functionalities in class vm::MeshOptimizer
 * \author Ramsharan Rangarajan
 */

#include <vm_mesh_optimizer.h>
#include <vm_utils.h>
#include <random>
#include <queue>

namespace vm
{
  // ------- overload 1 --------- //
  
  // relax a vertex
  std::pair<bool, double>
  MeshOptimizer::relax(const pmp::Vertex& vertex,
		       const QualityEvaluator &QE,
		       double qmin,
		       int num_samples)
  {
    assert(qmin>0.);
    
    const double curr_quality = QE(vertex, mesh);

    // needs improvement?
    if(curr_quality>qmin)
      return {false, curr_quality};
    
    // cannot move boundary vertices
    if(mesh.is_boundary(vertex)==true)
      return {false, curr_quality};

    // cannot move vertices on intefaces
    auto v_interface_ids = mesh.get_vertex_property<int>("interface_id");
    if(v_interface_ids[vertex]!=-1)
      return {false, curr_quality};
    
    // identify a feasible new position & move
    const auto result = compute_improved_vertex_position(vertex, num_samples, QE);
        
    // no feasible point
    if(std::get<bool>(result)==false)
      return {false, curr_quality};
    
    // found a feasible point.

    // is the quality improved sufficiently
    if(std::get<double>(result)>qmin)
      {
	mesh.position(vertex) = std::get<pmp::Point>(result);
	return {true, std::get<double>(result)};
      }
    else
      return {false, curr_quality};
  }


  // ------- overload 2 --------- //
  namespace {
    // alias
    using VQ_pair_t = std::pair<pmp::Vertex, double>;
  
    // Custom comparator of vertex/quality pairs
    bool PoorerVertexFirst(const VQ_pair_t& A, const VQ_pair_t& B)
    { return A.second>B.second; }
  }

  int MeshOptimizer::relax(const std::set<pmp::Vertex>& subset,
			   const QualityEvaluator& QE,
			   double qmin,
			   int num_samples,
			   const ProgressCallback &callback)
  {
    assert(qmin>0. && num_samples>=1);
    
    // tolerance for comparing qualities
    const double qeps = qmin/100.;

    // interface ids of vertices
    auto v_interface_ids = mesh.get_vertex_property<int>("interface_id");
    
    // priority queue of vertices to be relaxed during this iteration
    std::priority_queue<VQ_pair_t, std::vector<VQ_pair_t>, decltype(&PoorerVertexFirst)> vertex_queue(PoorerVertexFirst);
    for(auto& v:subset)
      if(mesh.is_boundary(v)==false &&
	 v_interface_ids[v]==-1)
	{
	  double qval = QE(v, mesh);
	  if(qval<qmin)
	    vertex_queue.push({v, qval});
	}
    const int qsize = static_cast<int>(vertex_queue.size());

    // #vertices relaxed during this iteration
    int nrelaxed = 0;

    // traverse the queue
    while(!vertex_queue.empty())
      {
	// pop the first vertex in the queue
	auto vq = vertex_queue.top();
	const auto& v = vq.first;
	vertex_queue.pop();

	// do nothing if:
	// vertex may have improved due to neighboring relaxations
	// the quality at this vertex, which could have changed due to other vertices moving, is > qmin
	const double curr_q = QE(v, mesh);
	if(curr_q>qmin)
	  continue;
	
	// reposition this vertex in the queue if its quality has changed
	if(std::abs(curr_q-vq.second)>qeps)
	  {
	    vertex_queue.push({vq.first,curr_q});
	    continue;
	  }

	// this vertex is the current priority
	auto result = this->relax(v, QE, qmin, num_samples);
	auto success = result.first;
	if(success==true)
	  {
	    ++nrelaxed;
	    if(callback!=nullptr)
	      {
		bool flag = callback(
				     {static_cast<int>(v.idx()),
					 qsize, nrelaxed,
					 std::get<double>(result)},
				     mesh, *this);

		// continue with relaxation
		if(flag==false)
		  return nrelaxed;
	      }
	  }
      }

    return nrelaxed;
  }


  // --------- overload 3 ---------- //
  
  int MeshOptimizer::relax(const QualityEvaluator &QE,
			   double qmin,
			   int num_samples,
			   const ProgressCallback &callback)
  {
    assert(qmin>0. && num_samples>=1);
    
    // all vertices
    auto v_container = mesh.vertices();
    std::set<pmp::Vertex> vertex_set{};
    for(auto v:v_container)
      vertex_set.insert(v);

    // agglomerate
    return relax(vertex_set, QE, qmin, num_samples, callback);
  }
  
  // ------ optimal relocation point calculation -------- //
  
  // identify a feasible point to move a vertex
  std::tuple<bool, pmp::Point, double>
  MeshOptimizer::compute_improved_vertex_position(const pmp::Vertex     &vertex,
						  const int             num_samples,
						  const QualityEvaluator &QE)  
  {
    assert(mesh.is_valid(vertex)==true);
    assert(mesh.is_boundary(vertex)==false);

    // given vertex position
    const pmp::Point given_vertex_pos = mesh.position(vertex);

    // get feasible sample points inside the visibility polygon
    const std::vector<std::pair<double,double>> feasible_samples =
      compute_feasible_vertex_positions(vertex, num_samples);
    
    // use the current vertex quality as the datum
    std::pair<double,double> curr_best_pos = {given_vertex_pos[0], given_vertex_pos[1]};
    double curr_best_quality = QE(vertex, mesh);
    
    // examine vertex qualities at the sample points
    pmp::Point& running_vert_pos = mesh.position(vertex);
    bool success = false;
    for(auto& sample:feasible_samples)
      {
	// move the vertex to this sample point's location
	running_vert_pos[0] = sample.first;
	running_vert_pos[1] = sample.second;
	
	// evaluate the resulting vertex quality
	double sample_quality = QE(vertex, mesh);
	
	// Does sample_quality dominate curr_best_quality
	if(sample_quality>curr_best_quality)
	  {
	    curr_best_quality = sample_quality;
	    curr_best_pos     = sample;
	    success           = true;
	  }
      }
    
    // restore the vertex position
    mesh.position(vertex) = given_vertex_pos;

    // done
    return {success, pmp::Point(curr_best_pos.first,curr_best_pos.second,given_vertex_pos[2]), curr_best_quality};
  }



  // ------ feasible relocation point generation -------- //
  namespace {
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
  }
    
  
  // random generation of feasible vertex positions
  std::vector<std::pair<double,double>>
  MeshOptimizer::compute_feasible_vertex_positions(const pmp::Vertex &vertex,
						   const int num_samples) const
  {
    // boost polygon of the environment around the vertex
    boost_polygon_t poly;
    auto vertex_ring = get_vertex_ring(vertex);
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
    samples.reserve(2*num_samples);

    // sample the bounding box
    std::uniform_real_distribution<> xdis(bg::get<0>(min_corner), bg::get<0>(max_corner));
    std::uniform_real_distribution<> ydis(bg::get<1>(min_corner), bg::get<1>(max_corner));
    for(int iter=0; iter<num_samples; ++iter)
      samples.push_back(boost_point_t(xdis(gen), ydis(gen)));

    // convex combinations of connected vertices not including "vertex" itself
    std::uniform_real_distribution<> lambda_dis(0.,1.);
    const int nconn = static_cast<int>(connected_vertices.size());
    std::vector<double> wts(nconn);
    double wsum;
    double pt[2];
    for(int iter=0; iter<num_samples; ++iter) {

      // weights
      wsum = 0.;
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
      
    // // sample incident edges
    // const double dlambda = 1./static_cast<double>(num_edge_samples+1);
    // const pmp::Point& Xv = mesh.position(vertex);
    // for(auto& Y:connected_vertices)
    //   {
    // 	double lambda = dlambda;
    // 	for(int iter=0; iter<num_edge_samples; ++iter)
    // 	{
    // 	  samples.push_back(boost_point_t(lambda*Xv[0]+(1.-lambda)*Y[0],
    // 					  lambda*Xv[1]+(1.-lambda)*Y[1]));
    // 	  lambda += dlambda;
    // 	}
    //   }
    
    // output = subset of feasible samples
    std::vector<std::pair<double,double>> feasible_points{};
    for(auto& sample:samples)
      if(feasibility_test_1(poly, connected_vertices, sample)==true &&
	 feasibility_test_2(mesh, vertex, sample)==true) {
	feasible_points.push_back({bg::get<0>(sample), bg::get<1>(sample)});
      }

    // done
    return feasible_points;
  }

}
