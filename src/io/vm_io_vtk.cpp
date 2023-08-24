// Sriramajayam

#include <vm_io.h>

namespace vm
{
  // Write a polygonal mesh in vtk file format
  // mesh [in]     : polygonal mesh
  // filename [in] : name of the file
  void write_vtk(const pmp::SurfaceMesh& mesh, const std::string filename)
  {
    assert(std::string(std::filesystem::path(filename).extension())==".vtk");

    std::fstream out;
    out.open(filename, std::ios::out);
    assert(out.good() && out.is_open());

    // Headers
    out << "# vtk DataFile Version 3.0" << std::endl;
    out << "Polygon mesh " << std::filesystem::path(filename).stem() << std::endl;
    out << "ASCII" << std::endl;
    out << "DATASET POLYDATA" << std::endl;

    // nodes
    auto v_container = mesh.vertices();
    out << "POINTS " << mesh.n_vertices() << " double" << std::endl;
    for(auto v:v_container)
      {
	const auto& X = mesh.position(v);
	out << X[0] << " " << X[1] << " " << X[2] << std::endl;
      }

    // polygons
    // compute the cell size = total number of integers in the list
    // keep track of the minimum cell vertex
    int cell_size = 0;
    auto f_circulator = mesh.faces();
    int min_vert_indx = 2;
    for(auto f:f_circulator)
      {
	cell_size += mesh.valence(f)+1;
	auto vface = mesh.vertices(f);
	for(auto v:vface)
	  if(v.idx()<min_vert_indx)
	    min_vert_indx = v.idx();
      }
    assert((min_vert_indx==0 || min_vert_indx==1) && "Expected vertex indexing to start from 0 or 1");

    out << "POLYGONS " << mesh.n_faces() << " " << cell_size << std::endl;
    for(auto f:f_circulator)
      {
	out << mesh.valence(f);
	auto vface = mesh.vertices(f);
	for(auto v:vface)
	  out << " " << v.idx()-min_vert_indx;
	out << std::endl;
      }

    // done
    out.close();
  }
      

  // Write a polygonal mesh in vtk file format with domain id
  // mesh [in]     : polygonal mesh
  // filename [in] : name of the file
  void write_vtk_with_cell_id(const pmp::SurfaceMesh& mesh, const std::string filename)
  {
    assert(mesh.has_face_property("id")==true);
    auto id_property = mesh.get_face_property<int>("id");
    
    // write the mesh in vtk format
    write_vtk(mesh, filename);
    
    // append face qualities as cell data
    std::fstream out;
    out.open(filename, std::ios::app);
    out << std::endl;
    out << "CELL_DATA " << mesh.n_faces() << std::endl;
    out << "SCALARS id int" << std::endl;
    out << "LOOKUP_TABLE default" << std::endl;
    auto f_circulator = mesh.faces();
    for(auto f:f_circulator)
      out << id_property[f] << std::endl;
  }
}
