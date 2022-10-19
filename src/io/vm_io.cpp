// Sriramajayam

#include <vm_io.h>
#include <fstream>
#include <cassert>

namespace vm
{

  // Reads a surface mesh
  void read_pmp(const std::string coord_file,
		const std::string conn_file,
		pmp::SurfaceMesh& mesh)
  {
    mesh.clear();
    
    // read vertex coordinates
    std::vector<pmp::Vertex> vertices{};
    std::fstream pfile;
    pfile.open(coord_file, std::ios::in);
    assert(pfile.good() && pfile.is_open());
    double xy[2];
    pfile >> xy[0];
    while(pfile.good())
      {
	pfile >> xy[1];
	vertices.push_back( mesh.add_vertex(pmp::Point(xy[0], xy[1], 0.)) );
	pfile >> xy[0];
      }
    pfile.close();
    std::cout << "Read " << mesh.n_vertices() << " vertices " << std::endl;
    
    // read triangle connectivities
    pfile.open(conn_file, std::ios::in);
    assert(pfile.good() && pfile.is_open());
    int conn[3];
    pfile >> conn[0];
    while(pfile.good())
      {
	pfile >> conn[1];
	pfile >> conn[2];
	mesh.add_triangle(vertices[conn[0]-1], vertices[conn[1]-1], vertices[conn[2]-1]);
	pfile >> conn[0];
      }
    pfile.close();
    std::cout << "Read " << mesh.n_faces() << " triangle " << std::endl;

    // done
    return;
  }


  // Writes a mesh in .off format
  void write_off(pmp::SurfaceMesh& mesh, const std::string filename)
  {
    const auto& positions = mesh.positions();
    std::fstream out;
    out.open(filename, std::ios::out);
    assert(out.good() && out.is_open());
    out << "OFF" << std::endl
	<< positions.size() << " " << mesh.n_faces() << " " << 0;

    // vertex coordinates
    for(auto& pt:positions)
      out << std::endl << pt[0] <<" " << pt[1] <<" " << pt[2];

    // connectivity
    auto face_circulator = mesh.faces();
    for(auto face:face_circulator)
      {
	out << std::endl << mesh.valence(face) << " ";
	auto vertex_circulator = mesh.vertices(face);
	for(auto v:vertex_circulator)
	  out << v.idx() <<" ";
      }
    out.close();
  }


  // Writes a given set of faces of a in .off format
  // Note that pmp::SurfaceMesh::write() does not corrrectly handle non-sequential vertex indexing
  // mesh [in]           : polygon mesh
  // filename [in]       : name of the file
  void write_off(pmp::SurfaceMesh& mesh,
		 const std::list<pmp::Face>& faces,
		 const std::string filename)
  {
    const auto& positions = mesh.positions();
    std::fstream out;
    out.open(filename, std::ios::out);
    assert(out.good() && out.is_open());
    out << "OFF" << std::endl
	<< positions.size() << " " << mesh.n_faces() << " " << 0;

    // vertex coordinates
    for(auto& pt:positions)
      out << std::endl << pt[0] <<" " << pt[1] <<" " << pt[2];

    // connectivity
    for(auto face:faces)
      {
	out << std::endl << mesh.valence(face) << " ";
	auto vertex_circulator = mesh.vertices(face);
	for(auto v:vertex_circulator)
	  out << v.idx() <<" ";
      }
    out.close();
  }
  
}
