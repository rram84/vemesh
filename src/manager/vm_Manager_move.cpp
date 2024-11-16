// Sriramajayam

#include <vm_Manager.h>
#include <vm_vertex_move.h>
#include <queue>
#include <iostream>

namespace vm
{
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

}
