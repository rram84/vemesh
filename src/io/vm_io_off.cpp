// Sriramajayam

#include <vm_io.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

namespace vm
{
  namespace {
    
    // Helper: trim leading and trailing whitespace
    inline std::string trim(const std::string &s)
    {
      size_t first = s.find_first_not_of(" \t\r\n");
      if (first == std::string::npos) return "";
      size_t last = s.find_last_not_of(" \t\r\n");
      return s.substr(first, last - first + 1);
    }

    // Helper: get next non-empty, non-comment line
    bool next_data_line(std::istream &in, std::string &line)
    {
      while (std::getline(in, line))
	{
	  line = trim(line);
	  if (!line.empty() && line[0] != '#')
	    return true;
	}
      return false;
    }
  }

  // Reads a .OFF mesh
  pmp::SurfaceMesh read_off(const std::string filename)
  {
    // Mesh
    pmp::SurfaceMesh mesh;
    
    // Extension check
    std::string ext = std::filesystem::path(filename).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext != ".off")
      throw std::runtime_error("Unexpected file extension: " + filename);

    // File exists
    if (!std::filesystem::exists(filename))
      throw std::runtime_error("File does not exist: " + filename);

    // Open file
    std::ifstream in(filename);
    if (!in.is_open() || !in.good())
      throw std::runtime_error("Could not open OFF file: " + filename);

    // Read header
    std::string header;
    if (!next_data_line(in, header) || header != "OFF")
      throw std::runtime_error("Invalid OFF file (missing OFF header): " + filename);

    // Read counts: num_vertices, num_faces, num_edges
    std::string counts_line;
    if (!next_data_line(in, counts_line))
      throw std::runtime_error("OFF file missing counts line: " + filename);
    std::istringstream counts_ss(counts_line);

    int num_vertices, num_faces, num_edges;
    if (!(counts_ss >> num_vertices >> num_faces >> num_edges))
      throw std::runtime_error("Failed to read vertex/face counts in OFF file: " + filename);
    if (num_vertices <= 0 || num_faces <= 0)
      throw std::runtime_error("Invalid number of vertices/faces in OFF file: " + filename);

    // Read vertices
    std::vector<pmp::Vertex> vertices(num_vertices);
    for (int i = 0; i < num_vertices; ++i)
      {
	std::string vline;
	if (!next_data_line(in, vline))
	  throw std::runtime_error("Unexpected end of file while reading vertices: " + filename);
	std::istringstream vs(vline);
	double x, y, z;
	if (!(vs >> x >> y >> z))
	  throw std::runtime_error("Failed to read vertex coordinates in OFF file: " + filename);

	vertices[i] = mesh.add_vertex(pmp::Point(x, y, z));
      }

    // Read faces
    for (int i = 0; i < num_faces; ++i)
      {
	std::string fline;
	if (!next_data_line(in, fline))
	  throw std::runtime_error("Unexpected end of file while reading faces: " + filename);
	std::istringstream fs(fline);

	int num_face_vertices;
	if (!(fs >> num_face_vertices))
	  throw std::runtime_error("Failed to read face vertex count in OFF file: " + filename);
	if (num_face_vertices < 3)
	  throw std::runtime_error("Face has fewer than 3 vertices in OFF file: " + filename);

	std::vector<pmp::Vertex> face_vertices(num_face_vertices);
	for (int j = 0; j < num_face_vertices; ++j)
	  {
	    int vidx;
	    if (!(fs >> vidx))
	      throw std::runtime_error("Failed to read face vertex index in OFF file: " + filename);
	    if (vidx < 0 || vidx >= num_vertices)
	      throw std::runtime_error("Face vertex index out of range in OFF file: " + filename);

	    face_vertices[j] = vertices[vidx];
	  }
	mesh.add_face(face_vertices);
      }

    // Final sanity check
    if (mesh.n_vertices() != num_vertices || mesh.n_faces() != num_faces)
      throw std::runtime_error("Mesh size mismatch after reading OFF file: " + filename);

    // Add default properties
    mesh.add_face_property<unsigned int>("domain_id", 1);
    mesh.add_vertex_property<int>("interface_id", -1);

    return mesh;
  }


  // Writes a mesh in .off format
  void write_off(const pmp::SurfaceMesh& mesh, const std::string filename)
  {
    // extension check
    std::string ext = std::filesystem::path(filename).extension();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if(!(ext == ".off"))
      throw std::runtime_error("Unexpected file extension: " + filename);

    // write
    std::fstream out;
    out.open(filename, std::ios::out);
    if (!out.is_open() || !out.good())
      throw std::runtime_error("Could not open file for writing: " + filename);

    // Header
    out << "OFF\n";
    out << mesh.n_vertices() << " " << mesh.n_faces() << " " << 0 << "\n";
    
    // vertex coordinates
    auto v_circulator = mesh.vertices();
    for(auto v:v_circulator)
      {
	const auto& X = mesh.position(v);
	out << X[0] << " " << X[1] << " " << X[2] << "\n";
      }

    // connectivity
    auto face_circulator = mesh.faces();
    for(auto face:face_circulator)
      {
	out << mesh.valence(face);
	auto vertex_circulator = mesh.vertices(face);
	for(auto v:vertex_circulator)
	  out << " " << v.idx();
	out << "\n";
      }
    out.close();

    if (!out.good())
      throw std::runtime_error("Error occurred while writing file: " + filename);
  }

}
