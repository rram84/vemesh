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
    :mesh(in_mesh)
  {
    // sanity checks
    assert(mesh.has_face_property("domain_id")==true);
    assert(mesh.has_vertex_property("interface_id")==true);
  }

  // visualize mesh along with face qualities
  void MeshOptimizer::evaluate_face_qualities(const QualityEvaluator& QE, std::string property_tag)
  {
    // face quality property
    if(mesh.has_face_property(property_tag)==false)
      {
	mesh.add_face_property<double>(property_tag);
      }

    // compute
    auto quality = mesh.get_face_property<double>(property_tag);
    auto f_circulator = mesh.faces();
    for(auto f:f_circulator)
      quality[f]= QE(f, mesh);

    // done
    return;
  }
  
  
  // visualize mesh along with vertex qualities
  void MeshOptimizer::evaluate_vertex_qualities(std::string face_quality_tag,
						std::string vertex_quality_tag)
  {
    // must have face quality tag
    assert(mesh.has_face_property(face_quality_tag)==true);
    
    // add vertex quality tag
    if(mesh.has_vertex_property(vertex_quality_tag)==false)
      {
	mesh.add_vertex_property<double>(vertex_quality_tag);
      }

    // access face and vertex qualities
    auto face_qualities = mesh.get_face_property<double>(face_quality_tag);
    auto vertex_qualities = mesh.get_vertex_property<double>(vertex_quality_tag);
    
    // compute vertex qualities
    auto v_circulator = mesh.vertices();
    double q;
    for(auto v:v_circulator)
      {
	double &min_q = vertex_qualities[v];
	
	// faces incident at this vertex
	auto f_circulator = mesh.faces(v);
	for(auto f:f_circulator)
	  {
	    q = face_qualities[f];
	    if(q < min_q)
	      min_q = q;
	  }
      }

    // done
    return;
  }

}
