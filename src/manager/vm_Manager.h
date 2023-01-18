// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <vm_quality.h>
#include <string>
#include <map>

namespace vm
{
  
  class Manager
  {
  public:
    //! Constructor
    Manager(const std::string off_file);
    
    //! Destructor
    virtual ~Manager();

    //! Disable copy and assignment
    Manager(const Manager&) = delete;
    Manager operator=(const Manager&) = delete;

    // inspect validity of the mesh
    void inspect_mesh() const;
    
    // access the mesh
    pmp::SurfaceMesh& get_mesh();

    // merge poor quality triangles, having one or more angles smaller than the tolerance provided, with neighbors
    // returns the number of merging operations performed
    int agglomerate_triangles(const double eps_degrees);
    
    // merge poor quality elements with neighbors
    // returns the number of merging operations performed
    int merge_faces(const double eps_degrees);

    // moves a vertex to a more favorable position
    std::pair<bool, LimitCircle_t> move_vertex(const pmp::Vertex& vertex, const int num_samples);

    // visualize
    void write(const std::string filename);

    // visualize elements with small angles
    void write_bad_angles(const std::string filename, const double eps_degrees);

    // visualize elements with small edges
    void write_bad_vertices(const std::string filename, const double eps_edge_ratio);
    
  private:
    pmp::SurfaceMesh mesh;
  };
}
