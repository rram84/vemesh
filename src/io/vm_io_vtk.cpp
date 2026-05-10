// Sriramajayam

/** \file vm_io_vtk.cpp
 * \brief Implementation of mesh I/O in VTK format
 * \author Ramsharan Rangarajan
 */

#include <vm_io.h>
#include <filesystem>
#include <fstream>
#include <limits>

namespace vm
{
  namespace {
    
    // Find the next line containing `word` (substring match, like before).
    // Returns the matched line via `out_line`.
    // On success, the stream is positioned at the start of the next line.
    bool find_line_with(std::istream& file,
			const std::string& word,
			std::string& out_line)
    {
      while (std::getline(file, out_line))
	if (out_line.find(word) != std::string::npos)
	  return true;
      return false;
    }
  }
    
  // read a vtk file
  pmp::SurfaceMesh read_vtk(const std::string filename)
  {
    // Mesh to return
    pmp::SurfaceMesh mesh;

    // sanity check on extension
    {
      std::string ext = std::filesystem::path(filename).extension().string();
      std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
      if (ext != ".vtk")
	throw std::runtime_error("Unexpected file extension: " + filename);
    }

    // open file for read
    std::ifstream file;
    file.open(filename);
    if (!file.is_open() || !file.good())
      throw std::runtime_error("Could not open VTK file: " + filename);
    
    // read # nodes
    int nNodes;
    {
      std::string line;
      if (!find_line_with(file, "POINTS", line))
	throw std::runtime_error("Could not find keyword POINTS");
      
      std::istringstream iss(line);
      std::string kw, datatype;
      
      if (!(iss >> kw >> nNodes >> datatype))
	throw std::runtime_error("Could not read #nodes");
    }
    
    // read coordinates
    double xyz[3];
    std::vector<pmp::Vertex> vertices{};
    for(int n=0; n<nNodes; ++n)
      {
	file >> xyz[0] >> xyz[1] >> xyz[2];
	vertices.push_back(mesh.add_vertex(pmp::Point(xyz[0], xyz[1], xyz[2])));
      }

    // read cells
    int nCells;
    std::vector<pmp::Face> faces{};
    {
      std::string line;
      if (!find_line_with(file, "POLYGONS", line))
	throw std::runtime_error("Could not find keyword POLYGONS");

      std::istringstream iss(line);
      std::string kw;
      if (!(iss >> kw >> nCells))
	throw std::runtime_error("Could not reads #cells");
    }
    
    for(int e=0; e<nCells; ++e)
      {
	int num_nodes;
	if(!(file >> num_nodes))
	  throw std::runtime_error("Failed to read num_nodes");
	    
	int node_num;
	std::vector<pmp::Vertex> conn(num_nodes);
	for(int a=0; a<num_nodes; ++a)
	  {
	    if(!(file >> node_num))
	      throw std::runtime_error("Failed to read vertex index");

	    // enforce sequential numbering
	    if(node_num < 0 || node_num >= vertices.size())
	      throw std::runtime_error("Vertex index out of bounds");
	    
	    conn[a] = vertices[node_num];
	  }
	auto f = mesh.add_face(conn);
	if(!f.is_valid())
	  throw std::runtime_error("Failed to create face");
	faces.push_back(f);
      }

    // domain ids of cells
    // default domain id = 0
    auto domain_ids = mesh.add_face_property<int>("domain_id", 0);
    {
      std::string line;
      if (find_line_with(file, "domain_id", line))
	{
	  std::getline(file, line);          // skip LOOKUP_TABLE line
	  for (auto f : faces)
	    {
	      int val;
	      if (!(file >> val))
		throw std::runtime_error("Failed to read domain_id");
	      domain_ids[f] = val;
	    }
	}
    }
    
    // vertices on interfaces, default interface id = -1
    auto interface_id = mesh.add_vertex_property<int>("interface_id", -1);
    {
      file.clear();
      file.seekg(0, std::ios::beg);
      std::string line;
      if (find_line_with(file, "interface_id", line))
	{
	  std::getline(file, line);          // skip LOOKUP_TABLE line
	  for (auto v : vertices)
	    {
	      int val;
	      if (!(file >> val))
		throw std::runtime_error("Failed to read interface_id");
	      interface_id[v] = val;
	    }
	}
    }
    
    // don't bother reading face/vertex qualities
    file.close();
    
    // done
    return mesh;
  }

  
  // Write a polygonal mesh in vtk file format
  // mesh [in]     : polygonal mesh
  // filename [in] : name of the file
  void write_vtk(const pmp::SurfaceMesh& mesh, const std::string filename)
  {
    // sanity check on file extension
    {
      std::string ext = std::filesystem::path(filename).extension().string();
      std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
      if (ext != ".vtk")
	throw std::runtime_error("Unexpected file extension: " + filename);
    }

    // open file for write
    std::fstream out;
    out.open(filename, std::ios::out);
    if (!out.is_open() || !out.good())
      throw std::runtime_error("Could not open VTK file: " + filename);

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
    int min_vert_indx = std::numeric_limits<int>::max();
    for(auto f:f_circulator)
      {
	cell_size += mesh.valence(f)+1;
	auto vface = mesh.vertices(f);
	for(auto v:vface)
	  if(v.idx()<min_vert_indx)
	    min_vert_indx = v.idx();
      }

    // empty mesh? otherwise check for 0/1 based indexing
    if (mesh.n_faces() == 0)
      min_vert_indx = 0;
    else if (min_vert_indx != 0 && min_vert_indx != 1)
      throw std::runtime_error("write_vtk: expected 0/1 based vertex indexing, got " + std::to_string(min_vert_indx));
    
    out << "POLYGONS " << mesh.n_faces() << " " << cell_size << std::endl;
    for(auto f:f_circulator)
      {
	out << mesh.valence(f);
	auto vface = mesh.vertices(f);
	for(auto v:vface)
	  out << " " << v.idx()-min_vert_indx;
	out << std::endl;
      }

    out << "CELL_DATA " << mesh.n_faces() << std::endl;

    // material ids
    if(!mesh.has_face_property("domain_id"))
      throw std::runtime_error("write_vtk: mesh should have property domain_id");
    
    auto dom_id = mesh.get_face_property<int>("domain_id");
    out << "SCALARS domain_id int" << std::endl
	<< "LOOKUP_TABLE default" << std::endl;
    for(auto f:f_circulator)
      {
	out << static_cast<int>(dom_id[f]) << std::endl;
      }
    
    // face qualities
    if(mesh.has_face_property(Face_Quality_Tag)==true)
      {
	auto face_quality = mesh.get_face_property<double>(Face_Quality_Tag);
	out << "SCALARS " << Face_Quality_Tag << " double" << std::endl
	    << "LOOKUP_TABLE default" << std::endl;
	for(auto f:f_circulator)
	  {
	    out << face_quality[f] << std::endl;
	  }
      }
    
    out << "POINT_DATA " << mesh.n_vertices() << std::endl;

    // interface id
    if(!mesh.has_vertex_property("interface_id"))
      throw std::runtime_error("write_vtk: mesh should have interface_id property");
    
    out << "SCALARS interface_id int" << std::endl
	<< "LOOKUP_TABLE default" << std::endl;
    auto interface_id = mesh.get_vertex_property<int>("interface_id");
    for(auto v:v_container)
      {
	out << interface_id[v] << std::endl;
      }
    
    // write vertex qualities if available
    if(mesh.has_vertex_property(Vertex_Quality_Tag)==true)
      {
	auto quality = mesh.get_vertex_property<double>(Vertex_Quality_Tag);
	out << "SCALARS " << Vertex_Quality_Tag << " double" << std::endl
	    << "LOOKUP_TABLE default" << std::endl;
	for(auto v:v_container)
	  {
	    out << quality[v] << std::endl;
	  }
      }
    
    // done
    out.close();
  }
      
}
