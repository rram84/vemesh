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
    bool merge_face(const pmp::Face& f, FaceQuality_f qfunc);

    // moves a vertex to a more favorable position
    std::pair<bool,double> move_vertex(const pmp::Vertex& vertex, const int num_samples, MeshVertexQuality_f qfunc);

    // visualize
    void write_mesh(const std::string filename);

    // visualize mesh along with qualities
    void write_mesh(const std::string filename, MeshFaceQuality_f qfunc);

    // visualize elements with small angles
    void write_bad_faces(const std::string filename, const double qeps, MeshFaceQuality_f qfunc);

    // visualize elements with small edges
    void write_bad_vertices(const std::string filename, const double qeps, MeshVertexQuality_f qfunc);
    
  private:
    pmp::SurfaceMesh mesh;
  };
}
