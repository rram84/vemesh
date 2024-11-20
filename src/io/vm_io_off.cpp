// Sriramajayam

#include <vm_io.h>
#include <filesystem>
#include <fstream>

namespace vm
{
  // Reads a .OFF mesh
  void read_off(const std::string filename, pmp::SurfaceMesh& mesh)
  {
    assert(std::filesystem::exists(filename)==true);
    const std::string extension = std::filesystem::path(filename).extension();
    assert(extension==".off" || extension==".OFF");
    std::fstream in;
    in.open(filename, std::ios::in);
    assert(in.good() && in.is_open());

    // first line: OFF
    std::string line_1;
    std::getline(in, line_1);
    
    // second line: # vertices, # faces, # edges
    int num_nodes, num_faces, num_edges;
    in >> num_nodes;
    in >> num_faces;
    in >> num_edges;
    assert(num_nodes>0 && num_faces>0 && num_edges==0);

    // read coordinates
    std::vector<pmp::Vertex> vertices(num_nodes);
    double xyz[3];
    for(int n=0; n<num_nodes; ++n)
      {
	in >> xyz[0];
	in >> xyz[1];
	in >> xyz[2];
	vertices[n] = mesh.add_vertex(pmp::Point(xyz[0], xyz[1], xyz[2])); 
      }

    // read faces
    int num_face_verts;
    int vnum;
    for(int e=0; e<num_faces; ++e)
      {
	in >> num_face_verts;
	assert(num_face_verts>=3);
	std::vector<pmp::Vertex> face_verts(num_face_verts);
	for(int a=0; a<num_face_verts; ++a)
	  {
	    in >> vnum;
	    assert(vnum>=0 && vnum<num_nodes);
	    face_verts[a] = vertices[vnum];
	  }
	mesh.add_face(face_verts);
      }

    in.close();
    
    // check
    assert(mesh.n_vertices()==num_nodes);
    assert(mesh.n_faces()==num_faces);

    // add default material id
    mesh.add_face_property<int>("material_id", 1);
    
    // add default interface id
    auto interface_id = mesh.add_vertex_property<int>("interface_id", 1);
    
    // done
    return;
  }


  // Writes a mesh in .off format
  void write_off(const pmp::SurfaceMesh& mesh, const std::string filename)
  {
    const std::string extension = std::filesystem::path(filename).extension();
    assert(extension==".off" || extension==".OFF");

    std::fstream out;
    out.open(filename, std::ios::out);
    assert(out.good() && out.is_open());
    out << "OFF" << std::endl
	<< mesh.n_vertices() << " " << mesh.n_faces() << " " << 0;

    // vertex coordinates
    auto v_circulator = mesh.vertices();
    for(auto v:v_circulator)
      {
	const auto& X = mesh.position(v);
	out << std::endl << X[0] << " " << X[1] << " " << X[2];
      }

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

}
