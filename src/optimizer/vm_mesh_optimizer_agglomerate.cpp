// Sriramajayam

/** \file vm_mesh_optimizer_agglomerate.cpp
 * \brief Implementation of element agglomeration functionalities in class vm::MeshOptimizer
 * \author Ramsharan Rangarajan
 */

#include <vm_mesh_optimizer.h>
#include <vm_mesh_inspection.h>
#include <list>
#include <queue>
#include <vector>
#include <iostream>

namespace vm
{
  // -----------  overload 1 ------ //
  
  // attempt to agglomerate a face with a neighbor
  std::tuple<bool, double, pmp::Face>
  MeshOptimizer::agglomerate(const pmp::Face& face,
			     const QualityEvaluator &QE,
			     double qfactor)
  {
    // sanity checks
    if (qfactor <= 1.)
      throw std::invalid_argument("MeshOptimizer::agglomerate: qfactor must be > 1");
    
    if (!mesh.is_valid(face) || mesh.is_deleted(face))
      throw std::invalid_argument("MeshOptimizer::agglomerate: face is invalid or deleted");

    // current quality
    const double curr_quality = QE(face, mesh);
    
    // return data
    std::tuple<bool, double, pmp::Face> ret_data{false, curr_quality, face};

    // no. merge along best possible neighbor
    auto result = find_halfedge_for_face_merge(face, QE);
    const auto& success       = std::get<bool>(result);
    const auto& best_quality  = std::get<double>(result);
    const auto& best_halfedge = std::get<pmp::Halfedge>(result);
    if(success==true &&
       best_quality>qfactor*curr_quality)
      {
	//ret_data = {success, best_quality, mesh.face(mesh.opposite_halfedge(best_halfedge))};
	auto agg_face = merge_neighbors(best_halfedge);
	ret_data = {success, best_quality, agg_face};
      }

    return ret_data;
  }


  // -----------  overload 2 ------ //

  namespace {
    // face-quality pairs
    using FQ_pair_t = std::pair<pmp::Face, double>;
  
    // Custom comparator of face/quality pairs
    bool PoorerFirst(const FQ_pair_t& A, const FQ_pair_t& B)
    { return A.second>B.second; }
  }
  
  // agglomerate faces in a subset
  int MeshOptimizer::agglomerate(const std::set<pmp::Face>& subset,
				 const QualityEvaluator& QE,
				 double qfactor,
				 const ProgressCallback &callback)
  {
    if (qfactor <= 1.)
      throw std::invalid_argument("MeshOptimizer::agglomerate: qfactor must be > 1");
    
    // tolerance for comparing qualities
    const double qeps = 1.e-6;

    // priority queue of faces to be merged during this iteration
    // priority_queue top() gives the face with lowest quality
    std::priority_queue<FQ_pair_t, std::vector<FQ_pair_t>, decltype(&PoorerFirst)> face_queue(PoorerFirst);
    for(auto f:subset)
      {
	double qval = QE(f, mesh);
	face_queue.push({f, qval});
      }
    const int qsize = static_cast<int>(face_queue.size());
      
    // track #of faces merged during execution
    int nmerged = 0;

    // traverse the queue
    while(!face_queue.empty())
      {
	// pop the first member in the queue
	auto fq = face_queue.top();
	const auto& f = fq.first;
	face_queue.pop();

	// do nothing if this face was erased during a merge
	if(mesh.is_deleted(f)==true)
	  continue;
	  
	// current quality
	const double curr_q = QE(f, mesh);
	
	// reposition this face in the queue if its quality has changed
	if(std::abs(curr_q-fq.second)>qeps)
	  {
	    face_queue.push({fq.first,curr_q});
	    continue;
	  }
	  
	// this face is the current priority
	auto result = this->agglomerate(f, QE, qfactor);
	auto success = std::get<bool>(result);
	if(success==true) {
	  ++nmerged;
	  if(callback!=nullptr)
	    {
	      bool flag = callback(
				   {static_cast<int>(std::get<pmp::Face>(result).idx()),
				       qsize, nmerged, std::get<double>(result)},
				   mesh, *this);
	      
	      // terminate agglomeration?
	      if(flag==false)
		return nmerged;
	    }
	}
      }
    
    return nmerged;
  }


  // ----- overload 3 ----- //
  
  // merge faces
  int MeshOptimizer::agglomerate(const QualityEvaluator &QE,
				 double qmin,
				 double qfactor,
				 const ProgressCallback &callback)
  {
    // sanity checks
    if (qmin <= 0.)
      throw std::invalid_argument("MeshOptimizer::agglomerate: qmin must be > 0");

    if (qfactor <= 1.)
      throw std::invalid_argument("MeshOptimizer::agglomerate: qfactor must be > 1");

    // evaluate face qualities in parallel
    const int nf = static_cast<int>(mesh.faces_size());
    std::vector<char> is_candidate(nf, 0);
#pragma omp parallel for schedule(dynamic)
    for(int i=0; i<nf; ++i)
      {
	const pmp::Face f(static_cast<pmp::IndexType>(i));
	if(!mesh.is_deleted(f))
	  {
	    if(QE(f,mesh)<qmin)
	      is_candidate[i] = 1;
	  }
      }

    // accumulate set faces
    std::set<pmp::Face> faceset{};
    for(int i=0; i<nf; ++i)
      if(is_candidate[i])
	faceset.insert(pmp::Face(static_cast<pmp::IndexType>(i)));
      
    return agglomerate(faceset, QE, qfactor, callback);
  }
  

  // ---- agglomerability condition ---- //
  // to determine agglomerable neighbors of a face along a given halfedge
  bool MeshOptimizer::is_agglomerable(const pmp::Halfedge& h) const
  {
    // validity
    if(!mesh.is_valid(h)) return false;
    auto f = mesh.face(h);
    if(!mesh.is_valid(f)) return false;
    
    // boundary
    if(mesh.is_boundary(mesh.edge(h))) return false;

    // isolated vertices
    if(mesh.valence(mesh.from_vertex(h))<=2) return false;
    if(mesh.valence(mesh.to_vertex(h))<=2)   return false;

    // neighbor
    auto nb_h = mesh.opposite_halfedge(h);
    if(!mesh.is_valid(nb_h)) return false;
    auto nb_f = mesh.face(nb_h);
    if(!mesh.is_valid(nb_f)) return false;

    // domain ids
    auto f_id = mesh.get_face_property<int>("domain_id")[f];
    auto nb_f_id = mesh.get_face_property<int>("domain_id")[nb_f];
    if(f_id!=nb_f_id) return false;

    // agglomerable
    return true;
  }
  
			 
  // ----- optimal agglomerable neighbor ----- //
  
  // identify the face along which to merge a given face
  std::tuple<bool, double, pmp::Halfedge>
  MeshOptimizer::find_halfedge_for_face_merge(const pmp::Face& face,
					      const QualityEvaluator &QE) const
  {
    // face needs to be merged with a neighbor
    // pick the agglomerable neighbor so that the resulting face has the best quality among all possibilities
    
    // evaluate halfedge merged -> resulting face quality
    pmp::Halfedge best_h;
    double best_quality = -1.;
    auto halfedge_circulator = mesh.halfedges(face);
    for(auto h:halfedge_circulator)
      if(is_agglomerable(h))
	{
	  auto nb_h = mesh.opposite_halfedge(h);
	  
	  // vertices of the new face created by merging face/nb_face
	  std::vector<pmp::Point> verts{};
	  
	  // vertices from the face of h
	  verts.push_back(mesh.position(mesh.to_vertex(h)));
	  auto it_h = mesh.next_halfedge(h);
	  while(it_h!=h)
	    {
	      verts.push_back(mesh.position(mesh.to_vertex(it_h)));
	      it_h = mesh.next_halfedge(it_h);
	    }
	  
	  // vertices from the face of nb_h
	  it_h = mesh.next_halfedge(nb_h);
	  while(it_h!=nb_h)
	    {
	      verts.push_back(mesh.position(mesh.to_vertex(it_h)));
	      it_h = mesh.next_halfedge(it_h);
	    }
	  verts.pop_back();
	  
	  // quality of the candidate merged face
	  //assert(inspect_face(verts)==true);
	  if(inspect_face(verts)==true)
	    {
	      double quality = QE(verts);
	      if(quality>best_quality)
		{
		  best_quality = quality;
		  best_h       = h;
		}
	    }
	}
    
    // there should be at least one candidate
    if(best_quality>0. && mesh.is_valid(best_h))
      return {true, best_quality, best_h};
    else
      return {false, best_quality, best_h};
  }
  

  // ---- execute merging across a halfedge
  // Agglomerate poor quality elements
  pmp::Face MeshOptimizer::merge_neighbors(const pmp::Halfedge& h0)
  {
    // sanity checks
    assert(mesh.has_face_property("domain_id")==true);
    assert(mesh.is_valid(h0) && !mesh.is_deleted(h0) && !mesh.is_boundary(h0));

    // face of h0
    auto f0 = mesh.face(h0);
    assert(mesh.is_valid(f0) && !mesh.is_deleted(f0));

    // opposite edge
    auto h1 = mesh.opposite_halfedge(h0);
    assert(mesh.is_valid(h1) && !mesh.is_deleted(h1) && !mesh.is_boundary(h1));

    // opposite face
    auto f1 = mesh.face(h1);
    assert(mesh.is_valid(f1) && !mesh.is_deleted(f1));

    // check on domain id
    auto domain_id = mesh.get_face_property<int>("domain_id");
    assert(domain_id[f0]==domain_id[f1]);
    
    const int nvertices = mesh.n_vertices();
    const int nprev_elm = mesh.n_faces();
    auto e = mesh.edge(h0);
    if (!mesh.is_removal_ok(e))
      throw std::runtime_error("MeshOptimizer::merge_neighbors: edge removal not permitted by mesh topology");
    if (!mesh.remove_edge(e))
      throw std::runtime_error("MeshOptimizer::merge_neighbors: remove_edge failed");

    // #vertices should remain unchanged
    assert(mesh.n_vertices()==nvertices);

    // #elements should reduce by 1
    assert(mesh.n_faces()==nprev_elm-1);

    // precisely one of f0 and f1 should be deleted
    assert(mesh.is_deleted(f0) || mesh.is_deleted(f1));
    assert(!(mesh.is_deleted(f0) && mesh.is_deleted(f1)));
    
    // done
    if(mesh.is_deleted(f0))
      return f1;
    else
      return f0;
  }

  
}
