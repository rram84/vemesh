// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <vm_face_quality.h>
#include <vm_vertex_quality.h>
#include <string>
#include <map>

namespace vm
{
  
  class Manager
  {
  public:
    //! Constructor
    Manager(const std::string off_file);

    //! Constructor
    Manager(const std::string node_file, const std::string ele_file);
    
    //! Destructor
    virtual ~Manager();

    //! Disable copy and assignment
    Manager(const Manager&) = delete;
    Manager operator=(const Manager&) = delete;

    // inspect validity of the mesh
    void inspect_mesh() const;
    
    // access the mesh
    pmp::SurfaceMesh& get_mesh();
    
    // merge a face with one of the neighbors
    // returns whether the face was merged or not, and the final quality
    std::pair<bool, pmp::Face> merge_face(const pmp::Face& f, FaceQuality_f qfunc);

    // moves a vertex to a more favorable position
    std::pair<bool,double> move_vertex(const pmp::Vertex& vertex, const int num_poly_samples, const int num_edge_samples, MeshVertexQuality_f qfunc);

    // visualize
    void write_mesh(const std::string filename);
    
    // visualize mesh along with face qualities
    void write_mesh(const std::string filename, MeshFaceQuality_f qfunc);

    // visualize mesh along with vertex qualities
    void write_mesh(const std::string filename, MeshVertexQuality_f qfunc);
    
  private:
    pmp::SurfaceMesh mesh;
  };
}
