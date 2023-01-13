// Sriramajayam

#include <vm_io.h>
#include <fstream>
#include <cassert>
#include <filesystem>
#include <sstream>

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

    // done
    return;
  }
  

  // Writes a mesh in .off format
  void write_off(pmp::SurfaceMesh& mesh, const std::string filename)
  {
    const std::string extension = std::filesystem::path(filename).extension();
    assert(extension==".off" || extension==".OFF");
    
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
    const std::string extension = std::filesystem::path(filename).extension();
    assert(extension==".off" || extension=="OFF");
    
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


  // Writes a mesh in .dat format, suitable for plotting with gnuplot
  void write_dat(const pmp::SurfaceMesh &mesh, const std::string filename)
  {
    const std::string extension = std::filesystem::path(filename).extension();
    assert(extension==".dat");

    std::fstream pfile;
    pfile.open(filename, std::ios::out);
    assert(pfile.good());
    
    auto f_circulator = mesh.faces();
    for(auto f:f_circulator)
      {
	auto v_circulator = mesh.vertices(f);
	for(auto v:v_circulator)
	  {
	    const auto& X = mesh.position(v);
	    pfile << X[0] << " " << X[1] << std::endl;
	  }
	// repeat the first vertex
	for(auto v:v_circulator)
	  {
	    const auto& X = mesh.position(v);
	    pfile << X[0] << " " << X[1] << std::endl;
	    break;
	  }
	
	pfile << std::endl;
      }
    pfile.close();
    return;    
  }
  

  // Writes a mesh in Sukumar's format
  void write_suku_format(const pmp::SurfaceMesh& mesh,
			 const std::string filename)
  {
    const std::string extension = std::filesystem::path(filename).extension();
    assert(extension.empty()==true);

    
    // Node file   : X, Y, boundary flag
    // Element file: material id, #edges, #nodes per edge, vertex ids

    // node file
    std::fstream pfile;
    pfile.open(filename+".node", std::ios::out);
    assert(pfile.good());
    const auto& v_circulator = mesh.vertices();
    for(auto v:v_circulator)
      {
	const auto& X = mesh.position(v); 
	bool bd_flag  = mesh.is_boundary(v);
	pfile << X[0] << "  " << X[1] << " " << static_cast<int>(bd_flag) << std::endl;
      }
    pfile.close();
    
    // element file
    const int mat_id = 1;
    pfile.open(filename+".ele", std::ios::out);
    assert(pfile.good());
    const auto& f_circulator = mesh.faces();
    for(auto f:f_circulator)
      {
	const int nedges = mesh.valence(f); // same as #vertices
	pfile << mat_id << " " << nedges << " " << 2 << " "; 
	const auto h0 = mesh.halfedge(f);
	pmp::Halfedge h = h0;
	while(true)
	  {
	    auto v0 = mesh.from_vertex(h);
	    auto v1 = mesh.to_vertex(h);
	    pfile << v0.idx() << " " << v1.idx() << " ";
	    h = mesh.next_halfedge(h);
	    if(h==h0)
	      break;
	  }
	pfile << std::endl;
      }
    pfile.close();

    // done
    return;
  }
}
