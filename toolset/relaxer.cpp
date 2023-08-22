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
#include <queue>
#include <utility>

// output directory management
void manage_output_directory(const std::string outdir);

// print element qualities
void write_element_qualities(const pmp::SurfaceMesh& mesh, const std::string filename);

// alias
using VQ_pair_t = std::pair<pmp::Vertex, double>;

// Custom comparator of vertex/quality pairs
bool Compare(const VQ_pair_t& A, const VQ_pair_t& B)
{ return A.second>B.second; }

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
  app.add_option("-n", num_iters, "number of agglomeration iterations")->required()->check(CLI::PositiveNumber);
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
  manager.write_mesh(outdir+"init.vtk", qfunc);
  auto& mesh = manager.get_mesh();
  write_element_qualities(mesh, outdir+"q-init.dat");
  
  // relaxation iterations
  for(int iter=0; iter<num_iters; ++iter)
    {
      // priority queue of vertices to be relaxed during this iteration
      std::priority_queue<VQ_pair_t, std::vector<VQ_pair_t>, decltype(&Compare)> vertex_queue(Compare);
      auto v_container = mesh.vertices();
      for(auto v:v_container)
	if(mesh.is_boundary(v)==false)
	  {
	    double qval = qfunc(mesh, v);
	    if(qval<qmin)
	      vertex_queue.push({v, qval});
	  }
      
      std::cout << "Iteration " << iter+1 << std::endl
		<<"#vertices marked for relaxation: " << vertex_queue.size() << std::endl;

      // #vertices relaxed during this iteration
      int nrelaxed = 0;

      // traverse the queue
      while(!vertex_queue.empty())
	{
	  // pop the first vertex in the queue
	  auto vq = vertex_queue.top();
	  const auto& v = vq.first;
	  vertex_queue.pop();

	  auto result = manager.move_vertex(v, 10, qfunc);
	  auto success = result.first;
	  if(success==true)
	    {
	      std::cout << "relaxing vertex: " << v.idx() << ", quality: " << vq.second <<" -> " << result.second << std::endl;
	      ++nrelaxed;
	      if(vis_flag && nrelaxed%10==0)
		{
		  manager.write_mesh(outdir+"mesh-i"+std::to_string(iter)+"-"+std::to_string(nrelaxed)+".vtk", qfunc);

		  // print element qualities
		  write_element_qualities(mesh, outdir+"q-"+std::to_string(nrelaxed)+".dat");
		}
	    }
	}
      std::cout << "#vertices relaxed: " << nrelaxed << std::endl << std::endl;

      // save output
      manager.write_mesh(outdir+"mesh-i"+std::to_string(iter)+".OFF");
      manager.write_mesh(outdir+"mesh-i"+std::to_string(iter)+".vtk", qfunc);
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
