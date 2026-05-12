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
		       int num_samples)
  {
    const double curr_quality = QE(vertex, mesh);

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
    
    // found a feasible point. update.
    mesh.position(vertex) = std::get<pmp::Point>(result);
    return {true, std::get<double>(result)};
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
			   int num_samples,
			   const ProgressCallback &callback)
  {
    if (num_samples < 1)
      throw std::invalid_argument("relax: num_samples must be >= 1");
    
    // tolerance for comparing qualities
    const double qeps = 1.e-8;

    // interface ids of vertices
    auto v_interface_ids = mesh.get_vertex_property<int>("interface_id");
    
    // priority queue of vertices to be relaxed during this iteration
    std::priority_queue<VQ_pair_t, std::vector<VQ_pair_t>, decltype(&PoorerVertexFirst)> vertex_queue(PoorerVertexFirst);
    for(auto& v:subset)
      if(mesh.is_boundary(v)==false &&
	 v_interface_ids[v]==-1)
	{
	  double qval = QE(v, mesh);
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

	// vertex may have improved due to neighboring relaxations
	// reposition this vertex in the queue if its quality has changed
	const double curr_q = QE(v, mesh);
	if(std::abs(curr_q-vq.second)>qeps)
	  {
	    vertex_queue.push({vq.first,curr_q});
	    continue;
	  }

	// this vertex is the current priority
	auto result = this->relax(v, QE, num_samples);
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
    if (qmin <= 0.)
      throw std::invalid_argument("relax: qmin must be > 0");
    if (num_samples < 1)
      throw std::invalid_argument("relax: num_samples must be >= 1");
    
    // all vertices
    auto v_container = mesh.vertices();
    
    // interface ids of vertices
    auto v_interface_ids = mesh.get_vertex_property<int>("interface_id");
    
    std::set<pmp::Vertex> vertex_set{};
    for(auto v:v_container)
      if(mesh.is_boundary(v)==false &&
	 v_interface_ids[v]==-1)
	{
	  double qval = QE(v, mesh);
	  if(qval<qmin)
	    vertex_set.insert(v);
	}

    // agglomerate
    return relax(vertex_set, QE, num_samples, callback);
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
    
    // Query whether a point is feasible: does point lie within the env polygon
    bool feasibility_test_1(const boost_polygon_t &poly,
			    const boost_point_t   &sample)
    {
      // does this point lie within the polygon
      return bg::within(sample, poly);
    }

    // Query whether a point is feasible: do all connected edges lie within the env polygon
    bool feasibility_test_2(const boost_polygon_t &poly,
			    const boost_point_t   &sample,
			    const std::vector<pmp::Point> &connected_verts)
    {
      // do the segments joining this point to the connected vertices
      // lie within the polygon?
      // Each Y is on the polygon boundary. nudge the segment's Y endpoint inward
      // by a small fraction (EPS) to make bg::within accept it.
      const double EPS = 0.01;
      for(auto& Y:connected_verts)
	{
	  boost_linestring_t seg;
	  bg::append(seg, sample);
	  bg::append(seg, boost_point_t(EPS*bg::get<0>(sample)+(1.-EPS)*Y[0],
					EPS*bg::get<1>(sample)+(1.-EPS)*Y[1]));
	  
	  if(bg::within(seg, poly)==false)
	    return false;
	}
      
      // done
      return true;
    }


    // Query if a point is feasible: are all perturbed faces ok
    bool feasibility_test_3(const pmp::SurfaceMesh &mesh,
			    const pmp::Vertex      &vertex,
			    const boost_point_t    &sample)
    {
      // faces incident at v
      auto face_circulator = mesh.faces(vertex);

      for(auto f:face_circulator)
	{
	  // this perturbed face
	  auto vert_circulator = mesh.vertices(f);
	  std::vector<pmp::Point> coords{};
	  for(auto w:vert_circulator)
	    {
	      if(w.idx()==vertex.idx())
		coords.push_back(pmp::Point(bg::get<0>(sample), bg::get<1>(sample), 0.));
	      else
		coords.push_back(mesh.position(w));
	    }
	  
	  // boost polygon
	  boost_polygon_t poly = make_polygon(coords);

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
    boost_polygon_t poly = get_environment_polygon(vertex, mesh);

    // axis-aligned bounding box for poly
    boost_box_t bbox;
    bg::envelope(poly, bbox);
    const auto& min_corner = bbox.min_corner();
    const auto& max_corner = bbox.max_corner();

    // connected vertices
    const std::vector<pmp::Point> connected_verts = get_connected_vertices(vertex, mesh);

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
    const int nconn = static_cast<int>(connected_verts.size());
    if(nconn==0)
      throw std::runtime_error("MeshOptimizer::compute_feasible_vertex_positions: found 0 1-ring neighbors");
    
    std::vector<double> wts(nconn);
    double wsum;
    double pt[2];
    for(int iter=0; iter<num_samples; ++iter) {

      // weights
      do {
	wsum = 0.;
	for(int i=0; i<nconn; ++i)
	  {
	    wts[i] = lambda_dis(gen);
	    wsum += wts[i];
	  }
      } while(wsum<=1.e-6);
      
      // normalize
      for(int i=0; i<nconn; ++i)
	wts[i] /= wsum;
      
      // sample point
      pt[0] = pt[1] = 0.;
      for(int i=0; i<nconn; ++i)
	{
	  const auto& X = connected_verts[i];
	  pt[0] += wts[i]*X[0];
	  pt[1] += wts[i]*X[1];
	}
      samples.push_back(boost_point_t(pt[0], pt[1]));
    }
      
    // output = subset of feasible samples
    std::vector<std::pair<double,double>> feasible_points{};
    for(auto& sample:samples)
      if( feasibility_test_1(poly, sample) &&
	  feasibility_test_2(poly, sample, connected_verts) &&
	  feasibility_test_3(mesh, vertex, sample) )
	feasible_points.push_back({bg::get<0>(sample), bg::get<1>(sample)});
    
    // done
    return feasible_points;
  }

}
