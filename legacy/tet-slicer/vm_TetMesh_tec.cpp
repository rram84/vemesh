// Sriramajayam

#include <vm_TetMesh.h>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <iostream>

namespace vm
{
  // positions cursor to get the data after 'word' occurs
  bool PositionCursor(std::istream &ifile, const std::string word)
  {
    ifile.seekg(0, std::ios_base::beg);
    while(ifile.good())
      {
	if(ifile.get() == word[0])
	  {
	    unsigned int ipos=1;
	    while(ipos!= word.length())
	      if(ifile.get() == word[ipos]) ipos++;
	      else  break;
	    if(ipos == word.length())
	      return true;
	  }
      }
    return false;
  };

  
  // read a tetrahedral mesh in tecplot format
  void TetMesh::read_tec(const std::string filename)
  {
    std::cout << "Reading tet mesh from " << filename << std::endl;
    assert(std::filesystem::exists(filename)==true);
    assert(std::string(std::filesystem::path(filename).extension())==".tec");

    std::fstream TecFile;
    TecFile.open(filename, std::ios::in);
    assert(TecFile.good() && TecFile.is_open());
    
    // Read  number of nodes:
    bool flag;
    flag = PositionCursor(TecFile, std::string("N="));
    assert(flag && "Could not read number of nodes");
    TecFile >> num_nodes;
  
    // Read number of elements:
    flag = PositionCursor(TecFile, std::string("E="));
    assert(flag && "Could not read number of elements");
    TecFile >> num_elements;
  
    // Read tet elements
    std::string ET;
    flag = PositionCursor(TecFile, std::string("ET="));
    assert(flag && "Could not read element type");
    TecFile>>ET;
    std::transform(ET.begin(), ET.end(), ET.begin(), ::toupper);
    assert(ET=="TETRAHEDRON");
    
    // Resize coordinates and connectivity vectors
    coordinates.resize(num_nodes);
    connectivity.resize(num_elements);
    
    // Read coordinates
    for(int a=0; a<num_nodes; ++a)
      {
	auto& xyz = coordinates[a];
	for(int k=0; k<3; ++k)
	  TecFile >> xyz[k];
      }

    // Read connectivity
    for(int e=0; e<num_elements; ++e)
      {
	auto& conn = connectivity[e];
	for(int a=0; a<4; ++a)
	  {
	    TecFile >> conn[a];
	    --conn[a];
	  }
      } 
    
    TecFile.close();
    std::cout << "Read " << num_nodes << " nodes and "
	      << num_elements << " elements " << std::endl;

    // done
    return;
  }

  // write a tet mesh in tecplot format
  void TetMesh::write_tec(const std::string filename) const
  {
    assert(std::string(std::filesystem::path(filename).extension())==".tec");
    assert(num_nodes>0 && num_elements>0);

    std::fstream TecFile;
    TecFile.open(filename, std::ios::out);
    assert(TecFile.good());
    TecFile.precision(16);
    TecFile.setf( std::ios::scientific );
  
    // Line 1:
    TecFile << "VARIABLES = \"X\", \"Y\", \"Z\" " << std::endl;
    
    // Line 2:
    TecFile << "ZONE t=\"t:0\", N="<< num_nodes << ", E=" << num_elements 
	    << ", F=FEPOINT, ET=TETRAHEDRON" << std::endl;
  
    // Nodal coordinates
    for(auto& X:coordinates)
      TecFile << X[0] << " " << X[1] << " " << X[2] << std::endl;
    
    // Element connectivity
    for(auto& conn:connectivity)
      TecFile << conn[0]+1 << " " << conn[1]+1 << " " << conn[2]+1 << " " << conn[3]+1 << std::endl;

    TecFile.close();

    // done
    return;
  }
  
}
