// Sriramajayam

#include <vm_manager.h>
#include <list>
#include <vm_snap.h>
#include <vm_io.h>

int main()
{
  vm::Manager manager("coordinates.dat", "connectivity.dat");
  manager.write("mesh.off");
  manager.write_bad_angles("bad_angles.off", 20.);
  manager.write_bad_vertices("bad_edges.off", 0.1);
  
  // merge
  int nmerged = manager.merge(20.);
  std::cout << "Merged "<<nmerged << " elements "<< std::endl;
  manager.write("merged-1.off");

  // merge again
  nmerged = manager.merge(20.);
  std::cout << "Merged "<<nmerged << " elements "<< std::endl;
  manager.write("merged-2.off");

  // faces with a snappable vertex
  auto& mesh = manager.get_mesh();
  auto face_circulator = mesh.faces();
  std::list<pmp::Face> snap_faces;
  snap_faces.clear();
  for(auto face:face_circulator)
    if(vm::needs_snap(mesh, face, 0.2).empty()==false)
      snap_faces.push_back(face);

  std::cout << "Number of snap faces: " << snap_faces.size() << std::endl; 
  vm::write_off(mesh, snap_faces, "snap-faces.off");

  // closest halfedge
  for(auto& face:snap_faces)
    {
      auto verts = vm::needs_snap(mesh, face, 0.2);
      assert(!verts.empty());
      for(auto& v:verts)
	{
	  auto h = vm::closest_halfedge(mesh, face, v);
	  std::cout << "Closest half edge to vertex "<< v.idx() <<": "
		    << mesh.from_vertex(h).idx()<<" --- " << mesh.to_vertex(h).idx() << std::endl;
	}
    }
}
