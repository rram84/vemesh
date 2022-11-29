// Sriramajayam

#include <vm_TetMesh.h>
#include <random>
#include <iostream>
#include <cmath>

namespace vm
{
  // constructor
  TetMesh::TetMesh()
    :connectivity{},
     coordinates{},
     num_nodes{0},
     num_elements{0}
  {}
  

  // perturb nodes away from a given z-plane
  int TetMesh::zperturb(const double zcoord, const double EPS)
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution d(0.5);
    
    int count = 0;
    for(auto& X:coordinates)
      if(std::abs(X[2]-zcoord)<EPS)
	{
	  if(d(gen)==true)
	    X[2] = zcoord+EPS;
	  else
	    X[2] = zcoord-EPS;

	  ++count;
	}

    // done
    return count;
  }
}
    
