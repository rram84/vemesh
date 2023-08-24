// Sriramajayam

#include <vm_io.h>
#include <fstream>
#include <cassert>
#include <filesystem>
#include <sstream>

namespace vm
{


  // Reads a mesh in triangle format
  void read_triangles(const std::string node_file, const std::string ele_file, pmp::SurfaceMesh& mesh)
  {
    assert(std::filesystem::exists(node_file)==true);
    assert(std::filesystem::exists(ele_file)==true);
    assert(std::filesystem::path(node_file).extension()==".node");
    assert(std::filesystem::path(ele_file).extension()==".ele");

    mesh.clear();
    
    // read coordinates from .node file
    // First line: <# of vertices> <dimension (must be 2)> <# of attributes> <# of boundary markers (0 or 1)>
    // Remaining lines: <vertex #> <x> <y> [attributes] [boundary marker]
    std::fstream pfile;
    pfile.open(node_file, std::ios::in);
    assert(pfile.good() && pfile.is_open());
    int nverts = 0;
    int spd    = 0;
    int num_node_attributes = 0;
    int nbd_descriptors     = 0;
    pfile >> nverts;
    pfile >> spd;
    pfile >> num_node_attributes;
    pfile >> nbd_descriptors;
    assert(nverts>=3                                  && "Expected at least three nodes in a triangle mesh");
    assert(spd==2                                     && "Expected spatial dimension to be 2");
    assert(num_node_attributes>=0                     && "Unexpected number of attributes");
    assert((nbd_descriptors==0 || nbd_descriptors==1) && "Unexpected number of boundary descriptors");

    // does node numbering start from 0 or 1?
    int nstart;
    pfile >> nstart;
    assert(nstart==0 || nstart==1);

    int vert_num = nstart;
    double xy[2];
    std::vector<pmp::Vertex> vertices(nverts);
    double attrib_val;
    int    bd_val;
    for(int i=0; i<nverts; ++i)
      {
	// vertices are expected in sequence
	assert(vert_num==i+nstart);
	
	// read coordinates
	pfile >> xy[0];
	pfile >> xy[1];
	vertices[i] = mesh.add_vertex(pmp::Point(xy[0], xy[1], 0.));

	// attributes
	for(int j=0; j<num_node_attributes; ++j)
	  pfile >> attrib_val;

	// boundary descriptor
	if(nbd_descriptors==1)
	  pfile >> bd_val;

	// next line
	pfile >> vert_num;
      }
    pfile.close();

    // read element connectivities from .ele file
    pfile.open(ele_file, std::ios::in);
    assert(pfile.good() && pfile.is_open());

    // First line: <# of triangles> <nodes per triangle> <# of attributes>
    // Remaining lines: <triangle #> <node> <node> <node> ... [attributes]
    int ntriangles;
    int num_nodes_per_triangle;
    int num_ele_attributes;
    int eindx;
    int verts[3];
    pfile >> ntriangles;
    pfile >> num_nodes_per_triangle;
    pfile >> num_ele_attributes;
    assert(ntriangles>=1 && "Expected mesh to have at least one triangle");
    assert(num_nodes_per_triangle==3 && "Expected 3 nodes per triangle");
    assert(num_ele_attributes>=0);
    for(int e=0; e<ntriangles; ++e)
      {
	pfile >> eindx;
	assert(eindx==e || eindx==e+1);
	for(int a=0; a<3; ++a)
	  {
	    pfile >> verts[a];
	    verts[a] -= nstart;
	  }
	for(int a=0; a<num_ele_attributes; ++a)
	  pfile >> attrib_val;

	// add face
	mesh.add_face({vertices[verts[0]], vertices[verts[1]], vertices[verts[2]]});
      }
    pfile.close();


    // done
    assert(mesh.n_vertices()==nverts);
    assert(mesh.n_faces()==ntriangles);

    // done
    return;
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


  // Write a triangle or quad mesh in tec file format
  // mesh [in]     : tri or quad mesh
  // filename [in] : name of the file
  void write_tec(const pmp::SurfaceMesh& mesh, const std::string filename)
  {
    assert(std::string(std::filesystem::path(filename).extension())==".tec");
    auto f_circulator = mesh.faces();
    int nvalence = 0;
    for(auto f:f_circulator)
      {
	nvalence = mesh.valence(f);
	break;
      }
    assert(nvalence==3 || nvalence==4);

    std::fstream outfile;
    outfile.open(filename, std::ios::out);
    assert(outfile.good());
    
    // Line 1:
    outfile<<"VARIABLES = \"X\", \"Y\" " << std::endl;

    // Line 2:
    const std::string ElmType = (nvalence==3) ? "TRIANGLE" :  "QUADRILATERAL";
    outfile<<"ZONE t=\"t:0\", N="<< mesh.n_vertices()
	   <<", E="<<mesh.n_faces()
	   <<", F=FEPOINT, ET="<<ElmType;

    // nodal coordinates
    auto v_circulator = mesh.vertices();
    for(auto v:v_circulator)
      {
	const auto& X = mesh.position(v);
	outfile << std::endl << X[0] << " " << X[1];
      }

    // element connectivities
    for(auto f:f_circulator)
      {
	auto conn = mesh.vertices(f);
	outfile << std::endl;
	for(auto v:conn)
	  outfile << v.idx()+1 << " "; 
      }
    outfile.close();

    // done
    return;
  }

  

  

  
}
