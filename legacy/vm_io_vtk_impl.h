// Sriramajayam

#pragma once

#include <fstream>

namespace vm
{
  // Write a polygonal mesh and cell face qualities in vtk file format
  // mesh  [in]     : polygonal mesh
  // qfunc [in]     : face quality function
  // filename [in]  : name of the file
  template<typename FuncType>
    void write_vtk_with_cell_data(const pmp::SurfaceMesh& mesh, FuncType func, const std::string filename)
    {
      // write the mesh in vtk format
      write_vtk(mesh, filename);

      // append face qualities as cell data
      std::fstream out;
      out.open(filename, std::ios::app);
      out << std::endl;
      out << "CELL_DATA " << mesh.n_faces() << std::endl;
      out << "SCALARS Q double" << std::endl;
      out << "LOOKUP_TABLE default" << std::endl;
      auto f_circulator = mesh.faces();
      for(auto f:f_circulator)
	out << func(mesh, f) << std::endl;

      // append material id as cell data
      if(mesh.has_face_property("id")==true)
	{
	  auto mat_id = mesh.get_face_property<int>("id");
	  out << std::endl
	      << "SCALARDS mat_id int" << std::endl
	      << "LOOKUP_TABLE default" << std::endl;
	  for(auto f:f_circulator)
	    out << mat_id[f] << std::endl;
	}

      // done
      return;
    }


  // Write a polygonal mesh and vertex qualities in vtk file format
  // mesh  [in]     : polygonal mesh
  // qfunc [in]     : vertex quality function
  // filename [in]  : name of the file
  template<typename FuncType>
    void write_vtk_with_vertex_data(const pmp::SurfaceMesh& mesh, FuncType func, const std::string filename)
    {
      // write the mesh in vtk format
      write_vtk(mesh, filename);

      // append vertex qualities
      std::fstream out;
      out.open(filename, std::ios::app);
      out << std::endl;
      out << "POINT_DATA " << mesh.n_vertices() << std::endl;
      out << "SCALARS Q double" << std::endl;
      out << "LOOKUP_TABLE default" << std::endl;
      auto v_circulator = mesh.vertices();
      for(auto v:v_circulator)
	out << func(mesh, v) << std::endl;

      return;
    }
}

