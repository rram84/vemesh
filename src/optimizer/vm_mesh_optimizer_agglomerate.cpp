// Sriramajayam

/** \file vm_mesh_optimizer_agglomerate.cpp
 * \brief Implementation of element agglomeration functionalities in class vm::MeshOptimizer
 * \author Ramsharan Rangarajan
 */

#include <vm_mesh_optimizer.h>
#include <vm_mesh_inspection.h>
#include <list>
#include <queue>
#include <iostream>

namespace vm
{
  // -----------  overload 1 ------ //
  
  // attempt to agglomerate a face with a neighbor
  std::tuple<bool, double, pmp::Face>
  MeshOptimizer::agglomerate(const pmp::Face& face,
			     const QualityEvaluator &QE,
			     double qmin,
			     double qfactor)
  {
    assert(qmin>0. && qfactor>1.);
    assert(mesh.is_valid(face) && !mesh.is_deleted(face));

    // current quality
    const double curr_quality = QE(face, mesh);
    
    // return data
    std::tuple<bool, double, pmp::Face> ret_data{false, curr_quality, face};

    // is quality already acceptable
    if(curr_quality>qmin)
      return ret_data;

    // no. merge along best possible neighbor
    auto result = find_halfedge_for_face_merge(face, QE);
    const auto& success       = std::get<bool>(result);
    const auto& best_quality  = std::get<double>(result);
    const auto& best_halfedge = std::get<pmp::Halfedge>(result);
    if(success==true &&
       best_quality>qmin &&
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
				 double qmin,
				 double qfactor,
				 const ProgressCallback &callback)
  {
    assert(qfactor>=1. && qmin>0.);
    
    // tolerance for comparing qualities
    const double qeps = qmin/100.;

    // priority queue of faces to be merged during this iteration
    // priority_queue top() gives the face with lowest quality
    std::priority_queue<FQ_pair_t, std::vector<FQ_pair_t>, decltype(&PoorerFirst)> face_queue(PoorerFirst);
    for(auto f:subset)
      {
	double qval = QE(f, mesh);
	if(qval<qmin)
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

	// do nothing if:
	// (i)  this face was erased during a merge
	// (ii) the quality of this face, which could have changed due to a merge, is > qmin
	if(mesh.is_deleted(f)==true)
	  continue;
	  
	// current quality
	const double curr_q = QE(f, mesh);
	if(curr_q>qmin)
	  continue;
	  
	// reposition this face in the queue if its quality has changed
	if(std::abs(curr_q-fq.second)>qeps)
	  {
	    face_queue.push({fq.first,curr_q});
	    continue;
	  }
	  
	// this face is the current priority
	auto result = this->agglomerate(f, QE, qmin, qfactor);
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
    assert(qmin>0. && qfactor>1.);
    std::set<pmp::Face> faceset{};
    auto f_container = mesh.faces();
    for(auto f:f_container)
      faceset.insert(f);

    return agglomerate(faceset, QE, qmin, qfactor, callback);
  }
  
  
  // ----- optimal agglomerable neighbor ----- //
  
  // identify the face along which to merge a given face
  std::tuple<bool, double, pmp::Halfedge>
  MeshOptimizer::find_halfedge_for_face_merge(const pmp::Face& face,
					      const QualityEvaluator &QE) const
  {
    assert(!mesh.is_deleted(face));
    assert(mesh.has_face_property("domain_id")==true);
    auto domain_id = mesh.get_face_property<int>("domain_id");
    const int my_domain_id = domain_id[face];
    
    // face needs to be merged with a neighbor
    // pick the neighbor so that the resulting face has the best quality among all possibilities
    // cannot merge along boundary faces
    // cannot merge along faces that would result in an isolated vertex
    // can merge with a face having the same "domain_id"

    
    // evaluate halfedge merged -> resulting face quality
    pmp::Halfedge best_h;
    double best_quality = -1.;
    auto halfedge_circulator = mesh.halfedges(face);
    for(auto h:halfedge_circulator)
      if(!mesh.is_boundary(mesh.edge(h)) &&                                         // no boundary merges
	 mesh.valence(mesh.from_vertex(h))>2 &&
	 mesh.valence(mesh.to_vertex(h))>2)  // prevent isolated vertices
	{
	  auto nb_h         = mesh.opposite_halfedge(h);
	  auto nb_face      = mesh.face(nb_h);
	  int  nb_domain_id = domain_id[nb_face];

	  if(mesh.is_valid(nb_h)       &&
	     mesh.is_valid(nb_face)    &&
	     !mesh.is_deleted(nb_face) &&
	     my_domain_id==nb_domain_id  )
	    {
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
    assert(mesh.is_removal_ok(e));
    bool flag = mesh.remove_edge(e);
    assert(flag==true);

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
