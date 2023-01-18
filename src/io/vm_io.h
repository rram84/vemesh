// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <list>
#include <map>

namespace vm
{
  // Reads a .OFF mesh
  void read_off(const std::string filename, pmp::SurfaceMesh& mesh);
  
  // Writes a mesh in .off format
  // Note that pmp::SurfaceMesh::write() does not correctly handle non-sequential vertex indexing
  // mesh [in]           : polygon mesh
  // filename [in]       : name of the file
  void write_off(pmp::SurfaceMesh& mesh, const std::string filename);

  // Writes a mesh in .dat format, suitable for plotting with gnuplot
  void write_dat(const pmp::SurfaceMesh &mesh, const std::string filename);

  // Writes a given set of faces of a in .off format
  // Note that pmp::SurfaceMesh::write() does not correctly handle non-sequential vertex indexing
  // mesh [in]           : polygon mesh
  // filename [in]       : name of the file
  void write_off(pmp::SurfaceMesh& mesh,
		 const std::list<pmp::Face>& faces,
		 const std::string filename);

  // Writes a mesh in Sukumar's format
  void write_suku_format(const pmp::SurfaceMesh& mesh,
			 const std::string filename);
    
}
