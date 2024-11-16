// Sriramajayam

#include <vm_Manager.h>
#include <vm_face_merge.h>
#include <queue>
#include <iostream>

namespace vm
{
  // merge poor quality elements with neighbors
  std::pair<bool, pmp::Face> Manager::merge_face(const pmp::Face& face, FaceQuality_f qface, MeshFaceQuality_f qfunc, const double qimprove_factor)
  {
    assert(mesh.is_valid(face) && !mesh.is_deleted(face));
    
    // merge along best possible neighbor
    auto result = find_halfedge_for_face_merge(mesh, face, qface);
    const auto& success       = std::get<0>(result);
    const auto& best_quality  = std::get<1>(result);
    const auto& best_halfedge = std::get<2>(result);
    const double curr_quality = qfunc(mesh, face);
    std::pair<bool, pmp::Face> ret_data{false, face};
    if(success==true && best_quality>qimprove_factor*curr_quality)
      {
	ret_data = {success, mesh.face(mesh.opposite_halfedge(best_halfedge))};
	vm::merge_face(mesh, best_halfedge);
      }

    return ret_data;
  }

  // face-quality pairs
  using FQ_pair_t = std::pair<pmp::Face, double>;
  
  // Custom comparator of face/quality pairs
  bool Compare(const FQ_pair_t& A, const FQ_pair_t& B)
  { return A.second>B.second; }
  
  // merge faces
  int Manager::merge_faces(MeshFaceQuality_f qfunc, FaceQuality_f qface,
			   const double qmin, const double qimprove_factor,
			   MeshUpdateCallback_f callback)
  {
    assert(qimprove_factor>=1.);
    
    // tolerance for comparing qualities
    const double qeps = qmin/100.;

    // priority queue of faces to be merged during this iteration
    std::priority_queue<FQ_pair_t, std::vector<FQ_pair_t>, decltype(&Compare)> face_queue(Compare);
    auto f_container = mesh.faces();
    for(auto f:f_container)
      {
	double qval = qfunc(mesh, f);
	if(qval<qmin)
	  face_queue.push({f, qval});
      }

    std::cout << "#faces marked for merge: " << face_queue.size() << std::endl;
      
    // track #of faces merged during this iteration
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
	// (ii) the quality of this face, which could have changed due to a merge, is > qEPS
	if(mesh.is_deleted(f)==true)
	  continue;
	  
	// current quality
	const double curr_q = qfunc(mesh, f);
	if(curr_q>qmin)
	  continue;
	  
	// reposition this face in the queue if its quality has changed
	if(std::abs(curr_q-fq.second)>qeps)
	  {
	    face_queue.push({fq.first,curr_q});
	    continue;
	  }
	  
	// this face is the current priority
	auto result = this->merge_face(f, qface, qfunc, qimprove_factor);
	auto success = result.first;
	if(success==true) {
	  ++nmerged;
	  if(callback!=nullptr)
	    callback(nmerged, mesh, *this);
	}
      }
    
    std::cout << "Merged " << nmerged << " faces" << std::endl;
    return nmerged;
  }
  
  
}
