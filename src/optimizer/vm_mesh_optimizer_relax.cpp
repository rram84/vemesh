// Sriramajayam

/** \file vm_mesh_optimizer_relax.cpp
 * \brief Implementation of vertex relaxation functionalities in class vm::MeshOptimizer
 * \author Ramsharan Rangarajan
 */

#include <vm_mesh_optimizer.h>
#include <vm_utils.h>
#include <random>
#include <queue>
#include <limits>
#include <algorithm>

#ifdef _OPENMP
#include <omp.h>
#endif


namespace vm
{
  // ------- overload 1 --------- //
  
  // relax a vertex
  std::pair<bool, double>
  MeshOptimizer::relax(const pmp::Vertex& vertex,
		       const QualityEvaluator &QE,
		       int num_samples,
		       std::optional<unsigned int> seed)
  {
    const double curr_quality = QE(vertex, mesh);

    // update seed for random num generator if provided
    if (seed) rng.seed(*seed);
    
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
			   const ProgressCallback &callback,
			   std::optional<unsigned int> seed)
  {
    if (num_samples < 1)
      throw std::invalid_argument("relax: num_samples must be >= 1");

    // update seed for random num generator if provided
    if (seed) rng.seed(*seed);
    
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
	auto result = this->relax(v, QE, num_samples); // if a seed was provided, rng was already reseeded
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
			   const ProgressCallback &callback,
			   std::optional<unsigned int> seed)
  {
    if (qmin <= 0.)
      throw std::invalid_argument("relax: qmin must be > 0");
    if (num_samples < 1)
      throw std::invalid_argument("relax: num_samples must be >= 1");

    // update random number generator seed if provided
    if(seed) rng.seed(*seed);
    
    // interface ids of vertices
    auto v_interface_ids = mesh.get_vertex_property<int>("interface_id");

    // evaluate vertex qualities in parallel
    const int nv = static_cast<int>(mesh.vertices_size());
    std::vector<char> is_candidate(nv, 0);
#pragma omp parallel for schedule(dynamic)
    for(int i=0; i<nv; ++i)
      {
	const pmp::Vertex v(static_cast<pmp::IndexType>(i));
	if(!mesh.is_deleted(v) && mesh.is_boundary(v)==false && v_interface_ids[v]==-1 && QE(v, mesh) < qmin)
	  is_candidate[i] = 1;
      }

    // accumulate vertex set
    std::set<pmp::Vertex> vertex_set{};
    for(int i=0; i<nv; ++i)
      if(is_candidate[i])
	vertex_set.insert(pmp::Vertex(static_cast<pmp::IndexType>(i)));
    
    // agglomerate
    return relax(vertex_set, QE, num_samples, callback); // if a seed was provided, rng was already reseeded
  }
  
  // ------ optimal relocation point calculation -------- //

  // dispatch: use the parallel candidate evaluation when built with OpenMP,
  // otherwise the serial one. Both yield identical results for a given seed.
  std::tuple<bool, pmp::Point, double>
  MeshOptimizer::compute_improved_vertex_position(const pmp::Vertex     &vertex,
						  const int             num_samples,
						  const QualityEvaluator &QE)
  {
#ifdef _OPENMP
    if(omp_get_max_threads() > 1)
      return compute_improved_vertex_position_parallel(vertex, num_samples, QE);
#endif
    return compute_improved_vertex_position_serial(vertex, num_samples, QE);
  }

  // --- serial implementation --- //
  std::tuple<bool, pmp::Point, double>
  MeshOptimizer::compute_improved_vertex_position_serial(const pmp::Vertex     &vertex,
							 const int             num_samples,
							 const QualityEvaluator &QE)  
  {
    assert(mesh.is_valid(vertex)==true);
    assert(mesh.is_boundary(vertex)==false);

    // given vertex position
    const pmp::Point given_vertex_pos = mesh.position(vertex);

    // Restore the vertex to its original position on every exit path
    // (normal return or exception), so a throw from QE cannot corrupt the mesh.
    struct PositionGuard {
      pmp::SurfaceMesh& mesh;
      pmp::Vertex       vertex;
      pmp::Point        saved;
      ~PositionGuard() { mesh.position(vertex) = saved; }
    } position_guard{mesh, vertex, given_vertex_pos};
    
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
    
    // position guard restores the vertex position in the mesh
    
    // done
    return {success, pmp::Point(curr_best_pos.first,curr_best_pos.second,given_vertex_pos[2]), curr_best_quality};
  }


  // --- parallel implementation --- //
  std::tuple<bool, pmp::Point, double>
  MeshOptimizer::compute_improved_vertex_position_parallel(const pmp::Vertex     &vertex,
							   const int             num_samples,
							   const QualityEvaluator &QE)
  {
    assert(mesh.is_valid(vertex)==true);
    assert(mesh.is_boundary(vertex)==false);

    // given vertex position (z carried through unchanged)
    const pmp::Point given_vertex_pos = mesh.position(vertex);

    // get feasible sample points inside the visibility polygon
    const std::vector<std::pair<double,double>> feasible_samples =
      compute_feasible_vertex_positions(vertex, num_samples);

    // Precompute, once, the incident-face polygons with a placeholder slot for `vertex`.
    // Each candidate is then scored on a local copy with that slot overwritten.
    // There is no shared-mesh mutation, so candidates are independent.
    std::vector<std::vector<pmp::Point>> face_coords;   // coords per incident face
    std::vector<std::size_t>             v_slot;        // index of `vertex` in each
    for(auto f : mesh.faces(vertex))
      {
	std::vector<pmp::Point> coords;
	std::size_t slot = 0, k = 0;
	for(auto w : mesh.vertices(f))
	  {
	    if(w == vertex) { slot = k; coords.push_back(given_vertex_pos); }
	    else            { coords.push_back(mesh.position(w)); }
	    ++k;
	  }
	face_coords.push_back(std::move(coords));
	v_slot.push_back(slot);
      }

    // vertex quality at an arbitrary (x,y): min over incident faces. Thread-safe:
    // reads only precomputed data and calls QE on local coordinates.
    auto quality_at = [&](double x, double y)
      {
	double q = std::numeric_limits<double>::max();
	for(std::size_t i = 0; i < face_coords.size(); ++i)
	  {
	    std::vector<pmp::Point> coords = face_coords[i];   // local copy
	    coords[v_slot[i]] = pmp::Point(x, y, 0.);
	    q = std::min(q, QE(coords));
	  }
	return q;
      };

    // evaluate all candidates in parallel
    const int nsamp = static_cast<int>(feasible_samples.size());
    std::vector<double> sample_quality(static_cast<std::size_t>(nsamp));
#pragma omp parallel for schedule(dynamic)
    for(int s = 0; s < nsamp; ++s)
      sample_quality[s] = quality_at(feasible_samples[s].first,
				     feasible_samples[s].second);

    // serial argmax: identical comparison order/tie-breaking to the serial loop,
    // so results are unchanged (parallelism is only in the QE evaluations above)
    std::pair<double,double> curr_best_pos = {given_vertex_pos[0], given_vertex_pos[1]};
    double curr_best_quality = quality_at(given_vertex_pos[0], given_vertex_pos[1]);
    bool success = false;
    for(int s = 0; s < nsamp; ++s)
      if(sample_quality[s] > curr_best_quality)
	{
	  curr_best_quality = sample_quality[s];
	  curr_best_pos     = feasible_samples[s];
	  success           = true;
	}

    return {success,
	    pmp::Point(curr_best_pos.first, curr_best_pos.second, given_vertex_pos[2]),
	    curr_best_quality};
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

      // Each connected vertex Y lies ON the environment polygon's boundary, so the
      // raw segment sample->Y is rejected by bg::within (which requires the strict
      // interior). We instead test the segment from sample to a point nudged a small
      // FRACTION (EPS) of the way from Y toward sample, pulling that endpoint just
      // inside the boundary.
      //
      // EPS is a fraction of the segment length, so this test is scale-invariant
      // (uniformly scaling the mesh does not change the outcome). EPS is a tolerance
      // for boundary/floating-point fuzz at the shared vertex -- NOT a mesh-scale or
      // edge-length parameter.
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

    // all samples
    std::vector<boost_point_t> samples{};
    samples.reserve(2*num_samples);

    // sample the bounding box
    std::uniform_real_distribution<> xdis(bg::get<0>(min_corner), bg::get<0>(max_corner));
    std::uniform_real_distribution<> ydis(bg::get<1>(min_corner), bg::get<1>(max_corner));
    for(int iter=0; iter<num_samples; ++iter)
      samples.push_back(boost_point_t(xdis(rng), ydis(rng)));

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
	    wts[i] = lambda_dis(rng);
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
