// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <array>
#include <vector>
#include <string>

namespace vm
{
  struct TetMesh
  {
    std::vector<std::array<int,4>>    connectivity;
    std::vector<std::array<double,3>> coordinates;
    int num_nodes;
    int num_elements;

    // constructor
    TetMesh();
    
    // read a tetrahedral mesh in tecplot format
    void read_tec(const std::string filename);

    // write a tetrahedral mesh in tecplot format
    void write_tec(const std::string) const;

    // perturb nodes away from a given z-plane
    int zperturb(const double zcoord, const double EPS);
    
    // slice a tet mesh at a z-plane
    // returns the number of perturbed nodes
    pmp::SurfaceMesh zslice(const double zcoord) const;
    
  };
}
