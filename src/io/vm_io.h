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

  // Writes a given set of faces of a in .off format
  // Note that pmp::SurfaceMesh::write() does not correctly handle non-sequential vertex indexing
  // mesh [in]           : polygon mesh
  // filename [in]       : name of the file
  void write_off(pmp::SurfaceMesh& mesh,
		 const std::list<pmp::Face>& faces,
		 const std::string filename);

  // VTK

  // Write a polygonal mesh in vtk file format
  // mesh [in]     : polygonal mesh
  // filename [in] : name of the file
  void write_vtk(const pmp::SurfaceMesh& mesh, const std::string filename);
  
  // Write a polygonal mesh in vtk file format
  // mesh [in]     : polygonal mesh
  // filename [in] : name of the file
  void write_vtk_with_cell_id(const pmp::SurfaceMesh& mesh, const std::string filename);
  
  // Write a polygonal mesh and cell face qualities in vtk file format
  // mesh  [in]     : polygonal mesh
  // qfunc [in]     : face quality function
  // filename [in]  : name of the file
  template<typename FuncType>
    void write_vtk_with_cell_data(const pmp::SurfaceMesh& mesh, FuncType func, const std::string filename);

  // Write a polygonal mesh and vertex qualities in vtk file format
  // mesh  [in]     : polygonal mesh
  // qfunc [in]     : vertex quality function
  // filename [in]  : name of the file
  template<typename FuncType>
    void write_vtk_with_vertex_data(const pmp::SurfaceMesh& mesh, FuncType func, const std::string filename);

  
  // suku
  
  // Writes a mesh in Sukumar's format
  void write_suku_format(const pmp::SurfaceMesh& mesh,
			 const std::string filename);
  
  // Reads a mesh in triangle format
  void read_triangles(const std::string node_file, const std::string ele_file, pmp::SurfaceMesh& mesh);
  
  // Writes a mesh in .dat format, suitable for plotting with gnuplot
  void write_dat(const pmp::SurfaceMesh &mesh, const std::string filename);
  
  // Write a triangle or quad mesh in tec file format
  // mesh [in]     : tri or quad mesh
  // filename [in] : name of the file
  void write_tec(const pmp::SurfaceMesh& mesh, const std::string filename);
  
}


// implementation of templated functions
#include <vm_io_vtk_impl.h>
