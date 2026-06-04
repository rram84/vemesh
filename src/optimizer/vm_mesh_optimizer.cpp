// Sriramajayam

/** \file vm_mesh_optimizer.cpp
 * \brief Implementation of class vm::MeshOptimzier
 * \author Ramsharan Rangarajan
 */

#include <vm_mesh_optimizer.h>
#include <vm_io.h>
#include <filesystem>

namespace vm
{
  // Constructor
  MeshOptimizer::MeshOptimizer(const pmp::SurfaceMesh& in_mesh)
    :mesh(in_mesh), rng(std::random_device{}())
  {
    // sanity checks
    if (!mesh.has_face_property("domain_id"))
      throw std::invalid_argument("MeshOptimizer: input mesh must have face property domain_id");
    
    if (!mesh.has_vertex_property("interface_id"))
      throw std::invalid_argument("MeshOptimizer: input mesh must have vertex property interface_id");
  }

  // visualize mesh along with face qualities
  void MeshOptimizer::evaluate_face_qualities(const QualityEvaluator& QE, const std::string& property_tag)
  {
    // face quality property
    if(mesh.has_face_property(property_tag)==false)
      {
	mesh.add_face_property<double>(property_tag);
      }

    // compute
    auto quality = mesh.get_face_property<double>(property_tag);
    const int nf = static_cast<int>(mesh.faces_size());
#pragma omp parallel for schedule(dynamic)
    for(int i=0; i<nf; ++i)
      {
	const pmp::Face f(static_cast<pmp::IndexType>(i));
	quality[f]= QE(f, mesh);
      }

    // done
    return;
  }
  
  
  // visualize mesh along with vertex qualities
  void MeshOptimizer::evaluate_vertex_qualities(const std::string& face_quality_tag,
						const std::string& vertex_quality_tag)
  {
    // must have face quality tag
    if (!mesh.has_face_property(face_quality_tag))
      throw std::invalid_argument("evaluate_vertex_qualities: mesh should have face property " + face_quality_tag);
    
    // add vertex quality tag
    if(mesh.has_vertex_property(vertex_quality_tag)==false)
      mesh.add_vertex_property<double>(vertex_quality_tag);
    
    // access face and vertex qualities
    auto face_qualities = mesh.get_face_property<double>(face_quality_tag);
    auto vertex_qualities = mesh.get_vertex_property<double>(vertex_quality_tag);
    
    // compute vertex qualities
    const int nv = static_cast<int>(mesh.vertices_size());
#pragma omp parallel for schedule(dynamic)
    for(int i=0; i<nv; ++i)
      {
	const pmp::Vertex v(static_cast<pmp::IndexType>(i));
	double min_q = std::numeric_limits<double>::max();
	
	// faces incident at this vertex
	auto f_circulator = mesh.faces(v);
	for(auto f:f_circulator)
	  {
	    const double q = face_qualities[f];
	    if(q < min_q)
	      min_q = q;
	  }
	vertex_qualities[v] = min_q;
      }

    // done
    return;
  }

}
