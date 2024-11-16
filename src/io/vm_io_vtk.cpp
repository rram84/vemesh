// Sriramajayam

#include <vm_io.h>
#include <filesystem>
#include <fstream>

namespace vm
{
  // function to position the cursor at a given word in a file
  bool position_cursor_after_word(std::ifstream& file, const std::string word)
  {
    bool found = false;
    std::string line;
    while (std::getline(file, line))
      {
	auto position = line.find(word);
	if ( position != std::string::npos)
	  {
	    file.seekg(-line.length()-1+position+word.length(), std::ios::cur);
	    found = true;
	    break;
	  }
      }
    // done
    return found;
  }
  
  // read a vtk file
  void read_vtk(const std::string filename, pmp::SurfaceMesh& mesh)
  {
    assert(std::string(std::filesystem::path(filename).extension())==".vtk");

    std::ifstream file;
    file.open(filename);
    assert(file.good() && file.is_open());
    
    // read # nodes
    bool flag = position_cursor_after_word(file, "POINTS");
    assert(flag==true);
    int nNodes;
    file >> nNodes;
      
    // read coordinates
    double xyz[3];
    std::string line;
    std::getline(file, line);
    std::vector<pmp::Vertex> vertices{};
    for(int n=0; n<nNodes; ++n)
      {
	file >> xyz[0] >> xyz[1] >> xyz[2];
	vertices.push_back(mesh.add_vertex(pmp::Point(xyz[0], xyz[1], xyz[2])));
      }

    // read cells
    std::vector<pmp::Face> faces{};
    flag = position_cursor_after_word(file, "POLYGONS");
    assert(flag==true);
    int nCells;
    file >> nCells;
    std::getline(file, line);
    for(int e=0; e<nCells; ++e)
      {
	int num_nodes;
	file >> num_nodes;
	int node_num;
	std::vector<pmp::Vertex> conn(num_nodes);
	for(int a=0; a<num_nodes; ++a)
	  {
	    file >> node_num;
	    conn[a] = vertices[node_num];
	  }
	faces.push_back(mesh.add_face(conn));
      }

    // material ids of cells
    auto material_ids = mesh.add_face_property<int>("material_id", -1);
    flag = position_cursor_after_word(file, "material_id");
    if(flag==true)
      {
	std::getline(file, line);
	std::getline(file, line);
	for(auto& f:faces)
	  {
	    file >> material_ids[f];
	  }
      }
    
    // vertices on interfaces
    auto interface_id = mesh.add_vertex_property<int>("interface_id", -1);
    file.seekg(0, std::ios::beg);
    flag = position_cursor_after_word(file, "interface_id");
    if(flag==true)
      {
	std::getline(file, line);
	std::getline(file, line);
	for(auto& v:vertices)
	  {
	    file >> interface_id[v];
	  }
      }
    
    // don't bother reading face/vertex qualities
    file.close();
    
    // done
    return;
  }

  
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

    out << std::endl << "CELL_DATA " << mesh.n_faces() << std::endl;

    // material ids
    assert(mesh.has_face_property("material_id")==true);
    auto mat_id = mesh.get_face_property<int>("material_id");
    out << "SCALARS material_id int" << std::endl
	<< "LOOKUP_TABLE default" << std::endl;
    for(auto f:f_circulator)
      {
	out << mat_id[f] << std::endl;
      }
    
    // face qualities
    if(mesh.has_face_property("face_quality")==true)
      {
	auto face_quality = mesh.get_face_property<double>("face_quality");
	out << "SCALARS face_quality double" << std::endl
	    << "LOOKUP_TABLE default" << std::endl;
	for(auto f:f_circulator)
	  {
	    out << face_quality[f] << std::endl;
	  }
      }
    
    out << std::endl << "POINT_DATA " << mesh.n_vertices() << std::endl;

    // interface id
    assert(mesh.has_vertex_property("interface_id")==true);
    out << "SCALARS interface_id int" << std::endl
	<< "LOOKUP_TABLE default" << std::endl;
    auto interface_id = mesh.get_vertex_property<int>("interface_id");
    for(auto v:v_container)
      {
	out << interface_id[v] << std::endl;
      }
    
    // write vertex qualities if available
    if(mesh.has_vertex_property("vertex_quality")==true)
      {
	auto quality = mesh.get_vertex_property<double>("vertex_quality");
	out << "SCALARS vertex_quality double" << std::endl
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
