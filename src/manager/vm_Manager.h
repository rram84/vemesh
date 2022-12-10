// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <string>
#include <map>

namespace vm
{
  
  class Manager
  {
  public:
    //! Constructor
    Manager(const std::string coord_file, const std::string conn_file);

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

    // merge poor quality elements with neighbors
    // returns the number of merging operations performed
    int merge_faces(const double eps_degrees);

    // snap vertices to nearby edges
    int snap_vertices(const double eps_dist_ratio);

    // moves vertices if the length of an incident edge is small
    int move_vertices(const double eps_len_ratio, const double eps_degrees, const int num_samples);

    // visualize
    void write(const std::string filename);

    // visualize elements with small angles
    void write_bad_angles(const std::string filename, const double eps_degrees);

    // visualize elements with small edges
    void write_bad_vertices(const std::string filename, const double eps_edge_ratio);
    
  private:
    pmp::SurfaceMesh mesh;
    std::map<pmp::Vertex, pmp::Vertex> curr2ref_vertex_map;
  };
}
