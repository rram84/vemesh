// Sriramajayam

// Agglomerate elements in a mesh based on quality

// Options
// -i Input mesh file in OFF format
// -o output directory, will be cleared if it already exists. will be created otherwise
// -q threshold tolerance
// -n number of agglomeration iterations to perform
// -v optional argument to print meshes after successive merges (within each iteration)

#include <vm_Manager.h>
#include <vm_io.h>
#include <vm_face_quality.h>
#include <CLI/CLI.hpp>
#include <queue>
#include <utility>

// output directory management
void manage_output_directory(const std::string outdir);

// callback for saving files
void MergeCallback(const std::string outdir, const int iter,
		   const int merge_num, const pmp::SurfaceMesh &mesh, vm::Manager &manager) {
  manager.write_mesh(outdir+"mesh-i"+std::to_string(iter)+"-"+std::to_string(merge_num)+".vtk");
  vm::write_suku_format(mesh, outdir+"/suku/mesh-i"+std::to_string(iter)+"-"+std::to_string(merge_num));
}

int main(int argc, char** argv)
{
  std::string meshfile;    // input mesh
  std::string outdir;      // output directory
  double qmin;             // quality threshold
  int  num_iters;          // iteration count
  bool vis_flag = false;   // detailed visualization
  
  // Command line options
  CLI::App app;
  app.add_option("-i", meshfile, "input mesh file in OFF format")->required()->check(CLI::ExistingFile);
  app.add_option("-o", outdir, "output directory. will be cleared if it exists")->required();
  app.add_option("-q", qmin, "quality threshold")->required()->check(CLI::PositiveNumber);
  app.add_option("-n", num_iters, "number of agglomeration iterations")->required()->check(CLI::PositiveNumber);
  app.add_flag("-v", vis_flag, "detailed mesh output after every merge, use sparingly");
  
  // parse
  CLI11_PARSE(app, argc, argv);
  assert(num_iters>=1);
  outdir += "/";

  // tolerance for comparing qualities
  const double qeps = qmin/100.;
  
  // output directory
  manage_output_directory(outdir);

  // Manager
  vm::Manager manager(meshfile);

  // mesh quality metric
  vm::MeshFaceQuality_f qfunc = vm::compute_stiffness_based_mesh_face_quality;
  vm::FaceQuality_f     qface = vm::compute_stiffness_based_face_quality;
  
  // initial mesh quality
  manager.compute_face_qualities(qfunc);
  manager.write_mesh(outdir+"init.vtk");
  auto& mesh = manager.get_mesh();
  vm::write_suku_format(mesh, outdir+"/suku/init");
      
  // iterations
  for(int iter=0; iter<num_iters; ++iter) {

    // callback
    auto callback = std::bind(MergeCallback, outdir, iter,
			      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    // merge faces
    manager.merge_faces(qfunc, qface, qmin, 1.2, callback);

    // save output
    manager.compute_face_qualities(qfunc);
    manager.write_mesh(outdir+"/off/mesh-i"+std::to_string(iter)+".OFF");
    manager.write_mesh(outdir+"/vtk/mesh-i"+std::to_string(iter)+".vtk");
    vm::write_suku_format(mesh, outdir+"/suku/mesh-i"+std::to_string(iter));
  }
  
  // done
}



// output directory management
void manage_output_directory(const std::string outdir)
{
  namespace fs = std::filesystem;
  
  // create the output directory if it does not exist, erase mesh files
  if(fs::exists(outdir))
    {
      std::cout << "Output directory " << outdir << " exists. Erasing mesh files " << std::endl;
      auto it_dir = fs::directory_iterator(outdir);
      for(auto& it:it_dir)
	{
	  const std::string ext = it.path().extension();
	  if(ext==".dat" || ext==".off" || ext==".OFF" || ext==".vtk")
	    {
	      auto flag = fs::remove(it.path());
	      assert(flag==true && "Could not erase file in output directory");
	    }
	}
    }
  else
    {
      std::cout << "Creating output directory \"" << outdir << "\"" << std::endl;
      auto flag  = fs::create_directory(outdir);
      assert(flag==true && "Could not create output directory");
      flag = fs::create_directory(outdir+"/vtk");
      assert(flag==true && "Could not create output directory");
      flag = fs::create_directory(outdir+"/off");
      assert(flag==true && "Could not create output directory");
      flag = fs::create_directory(outdir+"/suku");
      assert(flag==true && "Could not create output directory");
    }

  return;
}
