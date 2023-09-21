// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <list>
#include <map>

namespace vm
{
  // OFF
  
  // Reads a .OFF mesh
  void read_off(const std::string filename, pmp::SurfaceMesh& mesh);

  // Writes a mesh in .off format
  // mesh [in]           : polygon mesh
  // filename [in]       : name of the file
  void write_off(const pmp::SurfaceMesh& mesh, const std::string filename);
  
  // VTK

  // read a vtk file
  void read_vtk(const std::string filename, pmp::SurfaceMesh& mesh);
  
  // Write a polygonal mesh in vtk file format
  // mesh [in]     : polygonal mesh
  // filename [in] : name of the file
  void write_vtk(const pmp::SurfaceMesh& mesh, const std::string filename);
  
  // suku
  
  // Writes a mesh in Sukumar's format
  void write_suku_format(const pmp::SurfaceMesh& mesh, const std::string filename);

  // Write a mesh in Sukumar's format with element ids
  void write_suku_format_with_cell_id(const pmp::SurfaceMesh& mesh, const std::string filename);

  // GNUPLOT
  // Writes a mesh in .dat format, suitable for plotting with gnuplot
  void write_dat(const pmp::SurfaceMesh &mesh, const std::string filename);  
}
