// Sriramajayam

#include <vm_Manager.h>
#include <vm_io.h>
#include <vm_face_quality.h>
#include <set>

// compare pairs of (id, quality)
auto cmp = [](const std::pair<int,double>& a, const std::pair<int,double>& b)
{ return a.second<b.second; };

// print the poorest 10 face qualities
void print_poorest_faces(const std::set<std::pair<int,double>, decltype(cmp)>& qualities)
{
  int count = 0;
  for(auto& it:qualities)
    {
      if(count++>10) break;
      else std::cout << it.second << " ";
    }
}

int main()
{
  //vm::Manager manager("random_triangles.OFF");
  vm::Manager manager("ls-0p2859.OFF");
  //vm::Manager manager("slice.OFF");
  auto& mesh = manager.get_mesh();
  vm::write_suku_format(mesh, "ls-0p2859");

  // mesh quality metric
  vm::MeshFaceQuality_f qfunc = vm::compute_stiffness_based_mesh_face_quality;
  vm::FaceQuality_f     qface = vm::compute_stiffness_based_face_quality;

  // compute face qualities
  std::set<std::pair<int,double>, decltype(cmp)> q_pre(cmp);
  auto f_iterator = mesh.faces();
  for(auto f:f_iterator)
    q_pre.insert({f.idx(), qfunc(mesh,f)});

  // print the poorest 10 face qualities
  std::cout << std::endl << "Poorest 10 qualities: ";
  print_poorest_faces(q_pre);
  std::cout << std::endl;

  // threshold for element quality
  const double qeps = 0.1;

  // quality improvement factor
  const double qimprove_factor = 1.+1.e-6;
  
  // agglomerate poor quality faces
  for(auto f:f_iterator)
    if(!mesh.is_deleted(f) && mesh.is_valid(f))
      if(qfunc(mesh,f)<qeps)
	auto success = manager.merge_face(f, qface, qimprove_factor);

  // print mesh
  manager.write_mesh("merged.OFF");
  vm::write_suku_format(mesh, "ls-0p2859-agg");
  
  // recompute poorest 10 qualities
  std::set<std::pair<int,double>, decltype(cmp)> q_post(cmp);
  auto f_iterator_post = mesh.faces();
  for(auto f:f_iterator_post)
    q_post.insert({f.idx(), qfunc(mesh,f)});

  // print the poorest 10 face qualities
  std::cout << std::endl << "Poorest 10 qualities: ";
  print_poorest_faces(q_post);
  std::cout << std::endl;

}
