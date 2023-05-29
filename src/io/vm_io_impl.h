// Sriramajayam

#ifndef VM_IO_IMPL_H
#define VM_IO_IMPL_H

#include <fstream>

namespace vm
{
  // Write a polygonal mesh and cell face qualities in vtk file format
  // mesh  [in]     : polygonal mesh
  // qfunc [in]     : face quality function
  // filename [in]  : name of the file
  template<typename FuncType>
    void write_vtk(const pmp::SurfaceMesh& mesh, FuncType func, const std::string filename)
    {
      // write the mesh in vtk format
      write_vtk(mesh, filename);

      // append face qualities as cell data
      std::fstream out;
      out.open(filename, std::ios::app);
      out << std::endl;
      out << "CELL_DATA " << mesh.n_faces() << std::endl;
      out << "SCALARS" << " Q " << "double" << std::endl;
      out << "LOOKUP_TABLE default" << std::endl;
      auto f_circulator = mesh.faces();
      for(auto f:f_circulator)
	out << func(mesh, f) << std::endl;

      return;
    }
}

#endif

