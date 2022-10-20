// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <string>

namespace vm
{
  
  class Manager
  {
  public:
    //! Constructor
    Manager(const std::string coord_file, const std::string conn_file);

    //! Destructor
    virtual ~Manager();

    //! Disable copy and assignment
    Manager(const Manager&) = delete;
    Manager operator=(const Manager&) = delete;
    
    // merge poor quality elements with neighbors
    // returns the number of merging operations performed
    int merge(const double eps_degrees);

    // move vertices of small edges
    void move(const double eps_edge_ratio) const;

    // visualize
    void write(const std::string filename);

    // visualize elements with small angles
    void write_bad_angles(const std::string filename, const double eps_degrees);

    // visualize elements with small edges
    void write_bad_vertices(const std::string filename, const double eps_edge_ratio);

  private:

    // inspect validity of the mesh
    void inspect_mesh();

    // merge a face with a neighbor in the mesh
    void merge_face(const pmp::Face& face);
		    
    pmp::SurfaceMesh mesh;
  };
}
