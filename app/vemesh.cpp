// Sriramajayam

// Agglomerate elements and relax vertices in a mesh based on quality

// Flags: pick one
// -a agglomeration
// -r vertex relaxation
// -ar agglomeration + vertex relaxation
// -ra vertex relaxation + agglomeration

// Options
// -i Input polygonal mesh file in OFF/vtk format
// -o output directory, will be cleared if it already exists. will be created otherwise
// -q lower bound for acceptable element quality
// -n number of iterations to perform
// -s number of vertex samples in case of vertex relaxations
// -f quality improvement factor in case of agglomeration
// -v optional argument to print meshes after successive merges (within each iteration)

#include <vm_Manager.h>
#include <vm_face_quality.h>
#include <vm_vertex_quality.h>
#include <vm_io.h>
#include <CLI/CLI.hpp>

// validate command line options
std::map<std::string, CLI::Option*> validate_CLI_options(CLI::App &app);

// output directory management
void manage_output_directory(const std::string outdir);

// callback for saving files after each mesh update
void MeshUpdateCallback(const std::string outdir, const int iter, const std::string descr, const int index,
			const pmp::SurfaceMesh &mesh, vm::Manager &manager)
{
  manager.write_mesh(outdir+"/vtk/mesh-iter-"+std::to_string(iter)+"-"+descr+"-update-"+std::to_string(index)+".vtk");
  vm::write_suku_format(mesh, outdir+"/suku/mesh-iter-"+std::to_string(iter)+"-"+descr+"-update-"+std::to_string(index));
}


int main(int argc, char **argv)
{
  // Command line options
  CLI::App app;
  app.footer("Sample usage: \
             \n=================== \
             \n(i)   agglomerate elements:  ./vemesh -a -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -v \
             \n(ii)  relax vertices:        ./vemesh -r -i in_mesh.OFF -o out_dir -n 5 -s 25  \
             \n(iii) agglomerate and relax: ./vemesh --ar -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -s 25 -v \
             \n(iv)  relax and agglomerate: ./vemesh --ra -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -s 25  \n");

  // Flags
  app.add_flag("-a", "flag to agglomerate elements");
  app.add_flag("-r", "flag to relax mesh vertices");
  app.add_flag("--ar", "flag to agglomerate elements & relax vertices in that order at each iteration");
  app.add_flag("--ra", "flag to relax vertices & agglomerate elements in that order at each iteration");

  // Options
  std::string meshfile;    // input mesh
  std::string outdir;      // output directory
  double qthresh;          // quality threshold
  int  num_iters;          // iteration count
  bool vis_flag = false;   // detailed visualization
  int num_samples;         // number of sample points to generate
  double qfactor;          // element quality improvement factor
  app.add_option("-i", meshfile, "input mesh file in OFF/vtk format; should exist")->required()->check(CLI::ExistingFile);
  app.add_option("-o", outdir, "output directory; created if it does not exist; will be cleared if it does")->required();
  app.add_option("-n", num_iters, "number of iterations")->required()->check(CLI::PositiveNumber);
  app.add_option("-q", qthresh, "lower bound for acceptable element quality; elements with poorer qualities are marked for improvement")->required()->check(CLI::PositiveNumber);
  app.add_option("-f", qfactor, "minimum factor of improvement in element quality for agglomeration")->check(CLI::PositiveNumber);
  app.add_option("-s", num_samples, "number of random samples to generate for vertex relaxation")->check(CLI::PositiveNumber);
  app.add_flag("-v", vis_flag, "detailed mesh output after EVERY MESH UPDATE; leads to large num of file outputs; use sparingly");

  // parse options
  CLI11_PARSE(app, argc, argv);

  // validate  list of options
  const auto option_map = validate_CLI_options(app);
  app.parse(argc, argv);

  // expect at least one iteration
  assert(num_iters>=1);

  // output directory
  manage_output_directory(outdir);
  
  // Manager
  vm::Manager manager(meshfile);

  // initial mesh quality
  manager.compute_face_qualities(vm::compute_stiffness_based_mesh_face_quality);
  manager.write_mesh(outdir+"/vtk/init.vtk");
  auto& mesh = manager.get_mesh();
  vm::write_suku_format(mesh, outdir+"/suku/init");

  for(int iter=0; iter<num_iters; ++iter) {

    std::cout << "Iteration " << iter << std::endl;
    // callbacks
    auto callback_a = !(*option_map.at("v")) ? vm::MeshUpdateCallback_f(nullptr) : std::bind(MeshUpdateCallback, outdir, iter, "a", std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto callback_r = !(*option_map.at("v")) ? vm::MeshUpdateCallback_f(nullptr) : std::bind(MeshUpdateCallback, outdir, iter, "r", std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    
    // agglomeration only
    if(*option_map.at("a"))
      manager.merge_faces(vm::compute_stiffness_based_mesh_face_quality,
			  vm::compute_stiffness_based_face_quality,
			  qthresh, qfactor, callback_a);

    // relaxation only
    else if(*option_map.at("r"))
      manager.move_vertices(vm::compute_stiffness_based_vertex_quality,
			    qthresh, num_samples, std::sqrt(num_samples), callback_r);
    
    // agglomeration->relaxation
    else if(*option_map.at("ar")) {
      // agglomerate
      manager.merge_faces(vm::compute_stiffness_based_mesh_face_quality,
			  vm::compute_stiffness_based_face_quality,
			  qthresh, qfactor, callback_a);
      // relax
      manager.move_vertices(vm::compute_stiffness_based_vertex_quality,
			    qthresh, num_samples, std::sqrt(num_samples), callback_r);
    }

    // relaxation->agglomeration
    else if(*option_map.at("ra")) {
      // relax
      manager.move_vertices(vm::compute_stiffness_based_vertex_quality,
			    qthresh, num_samples, std::sqrt(num_samples), callback_r);
      // agglomerate
      manager.merge_faces(vm::compute_stiffness_based_mesh_face_quality,
			  vm::compute_stiffness_based_face_quality,
			  qthresh, qfactor, callback_a);
    }
    
    // save output at the end of this iteration
    manager.compute_face_qualities(vm::compute_stiffness_based_mesh_face_quality);
    manager.write_mesh(outdir+"/vtk/mesh-iter-"+std::to_string(iter)+".vtk");
    vm::write_suku_format(mesh, outdir+"/suku/mesh-iter-"+std::to_string(iter));
    std::cout << std::endl;

    // inspect the mesh
    manager.inspect_mesh();
  }
}
  

// validate command line options
std::map<std::string, CLI::Option*> validate_CLI_options(CLI::App& app) {

  // options
  auto options = app.get_options();
  std::map<std::string, CLI::Option*> option_map{};
  for(auto option:options)
    option_map.insert({option->get_single_name(), option});
  
  // operations provided
  const std::set<std::string> ops_strings{"a", "r", "ar", "ra"};
  std::string op_string;
  int num_ops = 0;
  for(auto& it:option_map) {
    if(*it.second) {
      if(ops_strings.find(it.first)!=ops_strings.end()) {
	op_string = it.first;
	++num_ops;
      } } }
  assert(num_ops==1);

  // chosen option
  auto& op = option_map.at(op_string);

  // common requirements for all options
  op->needs(option_map["o"]);
  op->needs(option_map["i"]);
  op->needs(option_map["q"]);
  op->needs(option_map["n"]);
  
  // // agglomeration
  if(op_string=="a" || op_string=="ar" || op_string=="ra")
    op->needs(option_map["f"]);

  // relaxation
  if(op_string=="r" || op_string=="ar" || op_string=="ra")
    op->needs(option_map["s"]);

  return option_map;
}


// output directory management
void manage_output_directory(const std::string outdir)
{
  namespace fs = std::filesystem;
  bool flag;
  
  // create the output directory if it does not exist, erase mesh files
  if(!fs::exists(outdir)) {
    flag = fs::create_directory(outdir); assert(flag);
    }
  
  // check for the vtk/ and suku subfolders
  bool has_vtk = false;
  bool has_suku = false;
  for(const auto& entry : fs::directory_iterator(outdir)) {
    if(entry.is_directory() && entry.path().filename()=="vtk")
      has_vtk = true;
    else if(entry.is_directory() && entry.path().filename()=="suku")
      has_suku = true;
  }
  
  if(!has_vtk) {
    flag = fs::create_directory(outdir+"/vtk"); assert(flag);
  }
  if(!has_suku) {
    flag = fs::create_directory(outdir+"/suku"); assert(flag);
  }
  
  // clear vtk files
  for (const auto& entry : fs::directory_iterator(outdir+"/vtk"))
    if (entry.is_regular_file() && entry.path().extension() == ".vtk")
      fs::remove(entry.path());
  
  // clear .node/.ele files
  for (const auto& entry : fs::directory_iterator(outdir+"/suku"))
    if (entry.is_regular_file() && (entry.path().extension() == ".node" || entry.path().extension() == ".ele"))
      fs::remove(entry.path());
  
  return;
}
