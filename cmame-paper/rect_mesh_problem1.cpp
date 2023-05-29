//********************************************************************************
//************************  Poisson Problem 1 (CMAME Paper) **********************
//********************************************************************************

#include <vm_Manager.h>
#include <vm_io.h>
#include <vm_MeshSlicer.h>
#include <vm_SpecialMeshes.h>
#include <vm_face_quality.h>
#include <CLI/CLI.hpp>
#include <set>

// output directory management
void manage_output_directory(const std::string outdir);

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


double level_set_segment(const double* X)
{
  //Line seg from (0,y1) to (1,y2): Equation of the line is; y = y1 + 0.5*(y2 - y1)*(x)
  const double y1 = 0.5;
  const double y2 = 0.5;
  return X[1] - y1 - 0.5*(y2 - y1)*X[0];
}


int main(int argc, char* argv[])
{
  // Command line options
  CLI::App app;

  // Options
  int nx=0, ny=0;
  app.add_option("--nx", nx, "#nodes along x")->required();
  app.add_option("--ny", ny, "#nodes along y")->required();
  std::string outdir = "./";
  app.add_option("-o", outdir, "Optional: output directory, defaulted to current directory. Erases contents if the directory already exists");
  bool flag_visualize = false;
  app.add_flag("-v", flag_visualize, "Optional: visualize the mesh during agglomeration iterations, defaulted to false");
  
  // parse
  CLI11_PARSE(app, argc, argv);
  assert(nx>=2 && ny>=2);

  // clear meshes in output directory if it exists, or create a new one
  outdir += "/";
  manage_output_directory(outdir);
  
  // construct quad mesh
  const double left_cnr[] = {0.,0.};
  const double hx = 1./static_cast<double>(nx-1);
  const double hy = 1./static_cast<double>(ny-1);
  auto rect_mesh = vm::create_rect_mesh(left_cnr, hx, nx, hy, ny);
  vm::write_off(rect_mesh, outdir+"rect.off");

  // clipping
  const double phi_eps  = 0.001;    // tolerance for |phi| > phi_eps.
  const double pert_eps = 0.005*hx; // step size for node perturbation
  vm::LevelSetFunction_t lsfunc = level_set_segment;
  auto nadjusted = vm::adjust_mesh_nodes(rect_mesh, phi_eps, pert_eps, lsfunc);
  std::cout << "Adjusted the position of " << nadjusted << " nodes away from the zero level set" << std::endl;
  vm::write_off(rect_mesh, outdir+"rect_adjusted.off");
  vm::clip_mesh(rect_mesh, 0.9*phi_eps, lsfunc);
  vm::write_off(rect_mesh, outdir+"clipped.OFF");

  // agglomeration
  vm::Manager manager(outdir+"clipped.OFF");
  auto& mesh = manager.get_mesh();
  vm::write_suku_format(mesh, outdir+"clipped");
    
  // mesh quality metric
  vm::MeshFaceQuality_f qfunc = vm::compute_stiffness_based_mesh_face_quality;
  vm::FaceQuality_f     qface = vm::compute_stiffness_based_face_quality;
  manager.write_mesh(outdir+"clipped.vtk", qfunc);
  
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
  
  // agglomerate poor quality faces
  int count = 0;
  int nMerged = 0;
  for(auto f:f_iterator)
    if(!mesh.is_deleted(f) && mesh.is_valid(f))
      if(qfunc(mesh,f)<qeps)
	{
	  count++;
	  
	  auto success = manager.merge_face(f, qface);
	  if(success==true)
	    ++nMerged;

	  // visualize
	  if(flag_visualize==true)
	    manager.write_mesh(outdir+"merged-"+std::to_string(count)+".vtk", qfunc);
	}
  std::cout << "#merges: " << nMerged << std::endl;
  
  // print mesh
  manager.write_mesh(outdir+"merged.vtk", qfunc);
  vm::write_suku_format(mesh, outdir+"merged_agg");
  
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


// output directory management
void manage_output_directory(const std::string outdir)
{
  // create the output directory if it does not exist, erase mesh files
  if(outdir!="./")
    {
      if(std::filesystem::exists(outdir))
	{
	  std::cout << "Output directory " << outdir << " exists. Erasing mesh files " << std::endl;
	  auto it_dir = std::filesystem::directory_iterator(outdir);
	  for(auto& it:it_dir)
	    {
	      const std::string ext = it.path().extension();
	      if(ext==".dat" || ext==".off" || ext==".OFF" || ext==".vtk")
		{
		  auto flag = std::filesystem::remove(it.path());
		  assert(flag==true && "Could not erase file in output directory");
		}
	    }
	}
      else
	{
	  std::cout << "Creating output directory \"" << outdir << "\"" << std::endl;
	  auto flag  = std::filesystem::create_directory(outdir);
	  assert(flag==true && "Could not create output directory");
	}
    }

  return;
}
