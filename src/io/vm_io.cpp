// Sriramajayam

#include <vm_io.h>
#include <fstream>
#include <cassert>

namespace vm
{
  // Reads a surface mesh
  void read_triangles(const std::string coord_file,
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
    std::cout << "Read " << mesh.n_vertices() << " vertices and "
	      << mesh.n_faces() << " triangles " << std::endl;

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
	<< positions.size() << " " << faces.size() << " " << 0;

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


  // Writes a mesh in .off format
  void write_off(const pmp::SurfaceMesh& mesh,
		 const std::map<pmp::Vertex, pmp::Vertex>& vertex_map,
		 const std::string filename)
  {
    const int nvertices = mesh.n_vertices();

    // map from new to old vertex references
    std::map<pmp::Vertex, pmp::Vertex> new_to_old_num_map{};
    auto vert_circulator = mesh.vertices();
    for(auto v:vert_circulator)
      new_to_old_num_map.insert({v, v});
    for(auto& it:vertex_map)
      {
	auto jt = new_to_old_num_map.find(it.first);
	assert(jt!=new_to_old_num_map.end());
	jt->second = it.second;
      }
    
    // vertex coordinates in order
    std::vector<pmp::Point> vertex_coords(nvertices);
    for(auto& it:new_to_old_num_map)
      vertex_coords[it.second.idx()] = mesh.position(it.first);  // number using old, position using new.

    std::fstream out;
    out.open(filename, std::ios::out);
    assert(out.good() && out.is_open());
    out << "OFF" << std::endl
	<< nvertices << " " << mesh.n_faces() << " " << 0;

    // write coordinates
    for(auto& pt:vertex_coords)
      out << std::endl << pt[0] <<" " <<pt[1] <<" " << 0.;

    // write connectivities
    auto face_circulator = mesh.faces();
    for(auto face:face_circulator)
      {
	out << std::endl << mesh.valence(face) <<" ";
	auto face_vertices = mesh.vertices(face);
	for(auto v:face_vertices)
	  out << new_to_old_num_map[v].idx() <<" ";
      }

    // done
    out.close();
    return;
  }
  
}
