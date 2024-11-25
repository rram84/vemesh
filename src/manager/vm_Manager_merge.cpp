// Sriramajayam

#include <vm_Manager.h>
#include <list>
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
	merge_face(mesh, best_halfedge);
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
    const int qsize = static_cast<int>(face_queue.size());
    std::cout << "#faces marked for merge: " << qsize << std::endl;
      
    // track #of faces merged during this iteration
    int nmerged = 0;
    int prev_percent = 0;

    // traverse the queue
    while(!face_queue.empty())
      {
	int percent_complete = (static_cast<int>(face_queue.size())*100)/qsize;
	if(percent_complete>prev_percent+20)
	  {
	    std::cout << "Progress: " << prev_percent+20 << "%" << std::endl;
	    prev_percent += 20;
	  }

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
    std::cout << "Progress: 100%" << std::endl;
    std::cout << "Merged " << nmerged << " faces" << std::endl;
    return nmerged;
  }


  
  
  // identify the face along which to merger a given face
  std::tuple<bool, double, pmp::Halfedge> Manager::find_halfedge_for_face_merge(const pmp::SurfaceMesh& mesh,
										const pmp::Face& face,
										FaceQuality_f qfunc)
  {
    assert(!mesh.is_deleted(face));
    assert(mesh.has_face_property("material_id")==true);
    auto material_id = mesh.get_face_property<int>("material_id");
    const int my_mat_id = material_id[face];
    
    // face needs to be merged with a neighbor
    // pick the neighbor so that the resulting face has the best quality among all possibilities
    // cannot merge along boundary faces
    // cannot merge along faces that would result in an isolated vertex
    // can merge with a face having the same "material_id"

    
    // evaluate halfedge merged -> resulting face quality
    pmp::Halfedge best_h;
    double best_quality = -1.;
    auto halfedge_circulator = mesh.halfedges(face);
    for(auto h:halfedge_circulator)
      if(!mesh.is_boundary(mesh.edge(h)) &&                                         // no boundary merges
	 mesh.valence(mesh.from_vertex(h))>2 && mesh.valence(mesh.to_vertex(h))>2)  // prevent isolated vertices
	{
	  auto nb_h      = mesh.opposite_halfedge(h);
	  auto nb_face   = mesh.face(nb_h);
	  int  nb_mat_id = material_id[nb_face];

	  if(mesh.is_valid(nb_h) && mesh.is_valid(nb_face) && !mesh.is_deleted(nb_face) && my_mat_id==nb_mat_id)
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
		  double quality = qfunc(verts);
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
  
  
  // Agglomerate poor quality elements
  void Manager::merge_face(pmp::SurfaceMesh& mesh, const pmp::Halfedge& h0)
  {
    // sanity checks
    assert(mesh.has_face_property("material_id")==true);
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

    // check on material id
    auto material_id = mesh.get_face_property<int>("material_id");
    assert(material_id[f0]==material_id[f1]);
    
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
    return;
  }

  
}
