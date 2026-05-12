// Sriramajayam

/** \file vm_io_quality_vectors.cpp
 * \brief Implementation of routines to save mesh quality vectors to file
 * \author Ramsharan Rangarajan
 */

#include <vm_io.h>
#include <algorithm>
#include <fstream>

namespace vm
{
  // helper to sort + write a index + vector to file
  namespace
  {
    void write_vector(std::vector<double> &vec,
		      const std::string filename)
    {
      // sort the vector in ascending order
      std::sort(vec.begin(), vec.end());

      // file
      std::fstream out;
      out.open(filename, std::ios::out);
      if(!out.is_open() || !out.good())
	throw std::runtime_error("Could not open file to write quality vector: "+filename);
    
      int count = 0;
      for(auto v:vec)
	out << ++count << "\t" << v << "\n";
      out.close();
    }
  }


  void write_face_quality_vector(const pmp::SurfaceMesh &mesh,
				 const std::string filename)
  {
    if(!mesh.has_face_property(Face_Quality_Tag))
      throw std::runtime_error("write_face_quality_vector: mesh does not have face_quality property");

    auto face_quality = mesh.get_face_property<double>(Face_Quality_Tag);
    auto f_circulator = mesh.faces();
    std::vector<double> qvec{};
    qvec.reserve(mesh.n_faces());
    for(auto f:f_circulator)
      qvec.push_back(face_quality[f]);

    write_vector(qvec, filename);
  }

  void write_vertex_quality_vector(const pmp::SurfaceMesh &mesh,
				   const std::string filename)
  {
    if(!mesh.has_vertex_property(Vertex_Quality_Tag))
      throw std::runtime_error("write_vertex_quality_vector: mesh does not have vertex_quality property");

    auto vertex_quality = mesh.get_vertex_property<double>(Vertex_Quality_Tag);
    auto v_circulator = mesh.vertices();
    std::vector<double> qvec{};
    qvec.reserve(mesh.n_vertices());
    for(auto v:v_circulator)
      qvec.push_back(vertex_quality[v]);

    write_vector(qvec, filename);
  }
}
