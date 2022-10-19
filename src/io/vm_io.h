// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <list>

namespace vm
{
  // Reads a surface mesh
  void read_triangles(const std::string coord_file, const std::string conn_file,
		      pmp::SurfaceMesh& mesh);
  
  // Writes a mesh in .off format
  // Note that pmp::SurfaceMesh::write() does not corrrectly handle non-sequential vertex indexing
  // mesh [in]           : polygon mesh
  // filename [in]       : name of the file
  void write_off(const pmp::SurfaceMesh& mesh, const std::string filename);

  // Writes a given set of faces of a in .off format
  // Note that pmp::SurfaceMesh::write() does not corrrectly handle non-sequential vertex indexing
  // mesh [in]           : polygon mesh
  // filename [in]       : name of the file
  void write_off(pmp::SurfaceMesh& mesh,
		 const std::list<pmp::Face>& faces,
		 const std::string filename); 
}
