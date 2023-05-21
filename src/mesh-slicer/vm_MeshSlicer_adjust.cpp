// Sriramajayam

#include <vm_MeshSlicer.h>
#include <cassert>
#include <map>

namespace vm
{
  int adjust_mesh_nodes(pmp::SurfaceMesh& mesh, const double phi_eps, const double pert_eps, LevelSetFunction_t& ls_func)
  {
    assert(phi_eps>0. && pert_eps>0.);
    auto v_container = mesh.vertices();
    double sd;
    int nadjusted = 0;
    for(auto v:v_container)
      {
	pmp::Point& X = mesh.position(v);
	double Y[3];
	for(int k=0; k<3; ++k)
	  Y[k] = X[k];
	sd = ls_func(Y);
	if(std::abs(sd)<phi_eps)
	  {
	    // adjust the node position until |phi| > phi_eps
	    int iter = 0.;
	    while(true)
	      {
		assert(iter<10 && "Exceeded 10 perturbations to adjust nodal position");
		
		// adjust the node position along the coordinate axes
		if(iter%2==0)
		  Y[0] += pert_eps;
		else
		  Y[1] += pert_eps;

		// recompute the level set function
		sd = ls_func(Y);
		if(std::abs(sd)>phi_eps)
		  break;
		else
		  ++iter;
	      }

	    // update node position
	    for(int k=0; k<3; ++k)
	      X[k] = Y[k];

	    // update the number of adjusted vertices.
	    ++nadjusted;
	  }
      }
    return nadjusted;
  }
}
