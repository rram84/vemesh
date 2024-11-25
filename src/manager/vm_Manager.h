// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <vm_face_quality.h>
#include <vm_vertex_quality.h>
#include <vm_inspect.h>
#include <string>
#include <tuple>
#include <map>

namespace vm
{
  class Manager;
  using MeshUpdateCallback_f = std::function<void(const int merge_num, const pmp::SurfaceMesh &mesh, Manager &manager)>;

  class Manager
  {
  public:
    //! Constructor
    Manager(const std::string filename);

    //! Destructor
    ~Manager() = default;

    //! Disable copy and assignment
    Manager(const Manager&) = delete;
    Manager operator=(const Manager&) = delete;

    // inspect validity of the mesh
    inline void inspect_mesh() const {
      assert(mesh.has_face_property("material_id")==true);
      assert(mesh.has_vertex_property("interface_id")==true);
      bool flag = vm::inspect_mesh(mesh); assert(flag && "Mesh is invalid");
    }
    
    // access the mesh
    inline pmp::SurfaceMesh& get_mesh()
    { return mesh; }
    
    // merge a face with one of the neighbors
    // returns whether the face was merged or not, and the final quality
    std::pair<bool, pmp::Face> merge_face(const pmp::Face& f, FaceQuality_f qface, MeshFaceQuality_f qfunc, const double qimprove_factor);

    // merge faces
    // returns the number of merged faces
    int merge_faces(MeshFaceQuality_f qfunc, FaceQuality_f qface, const double qthreshold, const double improve_factor, MeshUpdateCallback_f callback=nullptr);

    // moves a vertex to a more favorable position
    std::pair<bool,double> move_vertex(const pmp::Vertex& vertex, const int num_poly_samples,
				       const int num_edge_samples, MeshVertexQuality_f qfunc);

    // moves vertices
    int move_vertices(MeshVertexQuality_f qfunc, const double qthreshold,
		      const int num_poly_samples, const int num_edge_samples,
		      MeshUpdateCallback_f callback=nullptr);
    
     // compute face qualities in the mesh. saved under face property "quality"
    void compute_face_qualities(MeshFaceQuality_f qfunc);

    // compute vertex qualities in the mesh. saved under vertex property "quality"
    void compute_vertex_qualities(MeshVertexQuality_f qfunc);
    
    // visualize
    void write_mesh(const std::string filename);
       
    
  private:
    // identify the face along which to merger a given face
    static std::tuple<bool, double, pmp::Halfedge> find_halfedge_for_face_merge(const pmp::SurfaceMesh& mesh, const pmp::Face& face, FaceQuality_f qfunc);
    
    // Agglomerate faces along prescribed edge
    static void merge_face(pmp::SurfaceMesh& mesh, const pmp::Halfedge& halfedge);

    // identify a feasible point to move a vertex
    static std::tuple<bool, pmp::Point, double> compute_improved_vertex_position(pmp::SurfaceMesh          &mesh,
										 const pmp::Vertex         &vertex,
										 const int                 num_poly_samples,
										 const int                 num_edge_samples,
										 const MeshVertexQuality_f qfunc);
    // random generation of feasible vertex positions
    static std::vector<std::pair<double,double>>
      compute_feasible_vertex_positions(const pmp::SurfaceMesh& mesh,
					const pmp::Vertex&      vertex,
					const int               num_poly_samples,        // max number of random positions to generate
					const int               num_edge_samples);       // number of samples to generate per edge  
    pmp::SurfaceMesh mesh;
  };
}
