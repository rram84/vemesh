// Sriramajayam

// Relax interior nodes in a mesh based on quality

// Options
// -i Input mesh file in OFF format
// -o output directory, will be cleared if it already exists. will be created otherwise
// -q threshold tolerance
// -n number of relaxation iterations to perform
// -v optional argument to print meshes after successive merges (within each iteration)

#include <vm_Manager.h>
#include <vm_io.h>
#include <vm_vertex_quality.h>
#include <CLI/CLI.hpp>

// output directory management
void manage_output_directory(const std::string outdir);

// callback for saving files
void RelaxationCallback(const std::string outdir, const int iter,
			const int nmoved, const pmp::SurfaceMesh &mesh, vm::Manager &manager) {
  manager.write_mesh(outdir+"vtk/mesh-i"+std::to_string(iter)+"-"+std::to_string(nmoved)+".vtk");
  vm::write_suku_format(mesh, outdir+"/suku/mesh-i"+std::to_string(iter)+"-"+std::to_string(nmoved));
}


// print element qualities
void write_element_qualities(const pmp::SurfaceMesh& mesh, const std::string filename);

int main(int argc, char** argv)
{
  std::string meshfile; // input mesh
  std::string outdir;   // output directory
  double qmin;          // quality threshold
  int num_iters;        // iteration count
  bool vis_flag;        // detailed visualization

  // Command line options
  CLI::App app;
  app.add_option("-i", meshfile, "input mesh file in OFF format")->required()->check(CLI::ExistingFile);
  app.add_option("-o", outdir, "output directory. will be cleared if exists")->required();
  app.add_option("-q", qmin, "quality threshold")->required()->check(CLI::PositiveNumber);
  app.add_option("-n", num_iters, "number of relaxation iterations")->required()->check(CLI::PositiveNumber);
  app.add_flag("-v", vis_flag, "detailed mesh output after every perturbation. use for debugging only");

  // parse
  CLI11_PARSE(app, argc, argv);
  assert(num_iters>=1);
  outdir += "/";

  // output directory
  manage_output_directory(outdir);

  // Manager
  vm::Manager manager(meshfile);

  // mesh quality metric
  vm::MeshVertexQuality_f qfunc = vm::compute_stiffness_based_vertex_quality;
  vm::FaceQuality_f       qface = vm::compute_stiffness_based_face_quality;

  // initial mesh quality
  manager.compute_vertex_qualities(qfunc);
  manager.write_mesh(outdir+"init.vtk");
  auto& mesh = manager.get_mesh();
  write_element_qualities(mesh, outdir+"q-init.dat");
  vm::write_suku_format(mesh, outdir+"init");
  
  // relaxation iterations
  for(int iter=0; iter<num_iters; ++iter)
    {
      // callback
      auto callback = std::bind(RelaxationCallback, outdir, iter,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

      // relax vertices
      int nmoved = manager.move_vertices(qfunc, qmin, 50, 5, callback);
      
      // save output
      manager.compute_vertex_qualities(qfunc);
      manager.write_mesh(outdir+"/off/mesh-i"+std::to_string(iter)+".OFF");
      manager.write_mesh(outdir+"/vtk/mesh-i"+std::to_string(iter)+".vtk");
      vm::write_suku_format(mesh, outdir+"/suku/mesh-i"+std::to_string(iter));
    }
  
  // done
}


// output directory management
void manage_output_directory(const std::string outdir)
{
  // create the output directory if it does not exist, erase mesh files
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

  return;
}

// print element qualities
void write_element_qualities(const pmp::SurfaceMesh& mesh, const std::string filename)
{
  // compute element qualities
  std::vector<double> qvec{};
  auto faces = mesh.faces();
  for(auto f:faces)
    qvec.push_back(vm::compute_stiffness_based_mesh_face_quality(mesh, f));
  std::sort(qvec.begin(), qvec.end());
  
  std::fstream pfile;
  pfile.open(filename, std::ios::out);
  int indx = 1;
  for(auto& q:qvec)
    pfile << indx++ << "\t" << q << std::endl;
}
