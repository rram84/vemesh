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

// alias
using FQ_pair_t = std::pair<pmp::Face, double>;

// Custom comparator of face/quality pairs
bool Compare(const FQ_pair_t& A, const FQ_pair_t& B)
{ return A.second>B.second; }

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
  manager.write_mesh(outdir+"init.vtk", qfunc);
  auto& mesh = manager.get_mesh();

  // iterations
  for(int iter=0; iter<num_iters; ++iter)
    {
      // priority queue of faces to be merged during this iteration
      std::priority_queue<FQ_pair_t, std::vector<FQ_pair_t>, decltype(&Compare)> face_queue(Compare);
      auto f_container = mesh.faces();
      for(auto f:f_container)
	{
	  double qval = qfunc(mesh, f);
	  if(qval<qmin)
	    face_queue.push({f, qval});
	}

      std::cout << "Iteration " << iter+1 << std::endl
		<< "#faces marked for merge: " << face_queue.size() << std::endl;
      
      // # of faces merged during this iteration
      int nmerged = 0;

      // traverse the queue
      while(!face_queue.empty())
	{
	  // pop the first member in the queue
	  auto fq = face_queue.top();
	  const auto& f = fq.first;
	  face_queue.pop();

	  // do nothing if:
	  // (i)  this face was erased during a merge
	  // (ii) the quality of this face, which could have changed due to a merge, is > qEPS
	  if(mesh.is_deleted(f)==true)
	    continue;
	  
	  // current quality
	  const double curr_q = qfunc(mesh, f);
	  if(curr_q>qmin)
	    continue;
	  
	  // reposition this face in the queue if its quality has changed
	  if(std::abs(curr_q-fq.second)>qeps)
	    {
	      face_queue.push({fq.first,curr_q});
	      continue;
	    }
	  
	  // this face occupies the correct position in the queue
	  auto result = manager.merge_face(f, qface);
	  auto success = result.first;
	  if(success==true)
	    {
	      std::cout << "Merged " << f.idx() << " with " << result.second.idx() << std::endl;
	      ++nmerged;
	      if(vis_flag)
		manager.write_mesh(outdir+"mesh-i"+std::to_string(iter)+"-"+std::to_string(nmerged)+".vtk", qfunc);
	    }
	}
      std::cout << "#faces merged: " << nmerged << std::endl << std::endl;

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
