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
// -q bound for acceptable element quality
// -n number of iterations to perform
// -s number of vertex samples in case of vertex relaxations
// -f quality improvement factor in case of agglomeration
// -m face quality metric, 1:element stability ratio, 2:shape quality, 3:minimum angle
// -v optional argument to print meshes after successive merges (within each iteration)

#include <vm_MeshOptimizer.h>
#include <vm_face_quality.h>
#include <vm_io.h>
#include <CLI/CLI.hpp>

namespace fs = std::filesystem;

// optimization option
enum class OptOption {
  agglomerate_only,
  relax_only,
  agglomerate_then_relax,
  relax_then_agglomerate
};

// validate command line options
std::tuple<OptOption, bool> validate_CLI_options(CLI::App &app);

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
             \n(ii)  relax vertices:        ./vemesh -r -i in_mesh.OFF -o out_dir -n 5 -s 5  \
             \n(iii) agglomerate and relax: ./vemesh --ar -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -s 5 -v \
             \n(iv)  relax and agglomerate: ./vemesh --ra -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -s 5  \n");

  // Flags
  app.add_flag("-a", "flag to agglomerate elements");
  app.add_flag("-r", "flag to relax mesh vertices");
  app.add_flag("--ar", "flag to agglomerate elements & relax vertices in that order at each iteration");
  app.add_flag("--ra", "flag to relax vertices & agglomerate elements in that order at each iteration");

  // Options
  std::string meshfile;    // input mesh
  std::string outdir;      // output directory
  double qepsilon;         // quality threshold
  int  num_iters;          // iteration count
  bool vis_flag = false;   // detailed visualization
  int num_samples;         // number of sample points to generate
  double qfactor;          // element quality improvement factor
  int metric_num;          // 0:stability ratio, 1:shape quality, 2:min angle
  app.add_option("-i", meshfile, "input mesh file in OFF/vtk format; should exist")->required()->check(CLI::ExistingFile);
  app.add_option("-o", outdir, "output directory; created if it does not exist; will be cleared if it does")->required();
  app.add_option("-n", num_iters, "number of iterations")->required()->check(CLI::PositiveNumber);
  app.add_option("-q", qepsilon, "lower bound for acceptable element quality; elements with poorer qualities are marked for improvement")->required()->check(CLI::PositiveNumber);
  app.add_option("-f", qfactor, "minimum factor of improvement in element quality for agglomeration")->check(CLI::PositiveNumber);
  app.add_option("-s", num_samples, "number of random samples to generate for vertex relaxation")->check(CLI::PositiveNumber);
  app.add_option("-m", metric_num, "quality metric 1=element stability ratio, 2=shape quality, 3=min angle")->check(CLI::PositiveNumber);
  app.add_flag("-v", vis_flag, "detailed mesh output after every mesh update; leads to large num of file outputs; use sparingly");

  // parse options
  CLI11_PARSE(app, argc, argv);

  // validate  list of options
  const auto options = validate_CLI_options(app);
  app.parse(argc, argv);

  // expect at least one iteration
  assert(num_iters>=1);

  // output directory
  manage_output_directory(outdir);

  // Input mesh
  const std::string extension = fs::path(meshfile).extension().str();
  pmp::SurfaceMesh in_mesh;
  if(extension==".off" || extension==".OFF")
    in_mesh = vm::read_off(meshfile);
  else if(extension==".vtk" || extension==".VTK")
    in_mesh = vm::read_vtk(meshfile);
  else
    throw std::runtime_error("Expected mesh format to be off/vtk, given "+meshfile);
  
  // Mesh optimizer
  vm::MeshOptimizer optimizer(in_mesh);
  const auto& mesh = optimizer.get_mesh();

  // Face quality metric
  static const std::array<vm::FaceQualityFn, 3> all_quality_metrics = {
    vm::quality::vem_stability_ratio, // VEM element stability ratio
    vm::quality::geom_shape_ration,   // shape quality metric
    vm::quality::geom_min_angle       // min included angle
  };
  const auto face_quality_metric = all_quality_metrics[metric_num-1];

  // Quality evaluator
  vm::QualityEvaluator QE(face_quality_metric);
  
  // initial mesh quality
  const std::string qf_tag = "face_quality";
  const std::string qv_tag = "vertex_quality";
  optimizer.evaluate_face_qualities(qf_tag, QE);
  optimizer.evaluate_vertex_qualities(qf_tag, qe_tag);

  // save mesh with qualities
  vm::write_vtk(mesh, "input_mesh.vtk");

  // options provided
  const auto& opt_option = std::get<OptOption>(options);
  const bool verbose_flag = std::get<bool>(options);

  for(int iter=0; iter<num_iters; ++iter) {

    std::cout << "Mesh improvement iteration " << iter <<":\n" << std::flush;

    // optimize
    switch(opt_option)
      {
      case OptOption::agglomerate_only:
	{
	  optimizer.agglomerate(QE, qepsilon, qfactor);
	  break;
	}

      case OptOption::relax_only:
	{
	  optimizer.relax(QE, qepsilon, qfactor);
	  break;
	}

      case OptOption::agglomerate_then_relax:
	{
	  optimizer.agglomerate(QE, qepsilon, qfactor);
	  optimizer.relax(QE, qepsilon, qfactor);
	  break;
	}

      case OptOption::relax_then_agglomerate:
	{
	  optimizer.relax(QE, qepsilon, qfactor);
	  optimizer.agglomerate(QE, qepsilon, qfactor);
	  break;
	}
      }
    
    // callbacks
    //auto callback_a = !(*option_map.at("v")) ? vm::MeshUpdateCallback_f(nullptr) : std::bind(MeshUpdateCallback, outdir, iter, "a", std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    //auto callback_r = !(*option_map.at("v")) ? vm::MeshUpdateCallback_f(nullptr) : std::bind(MeshUpdateCallback, outdir, iter, "r", std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    
    // save output    
    optimizer.evaluate_face_qualities(QE, qf_tag);
    optimizer.evaluate_vertex_qualities(qf_tag, qv_tag);
    vm::write_vtk(mesh, "output.vtk");
    
    // inspect the mesh
    //manager.inspect_mesh();
  }
}
  

// validate command line options
std::tuple<OptOption, bool> validate_CLI_options(CLI::App &app) {

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
  op->needs(option_map["m"]);
  
  // // agglomeration
  if(op_string=="a" || op_string=="ar" || op_string=="ra")
    op->needs(option_map["f"]);

  // relaxation
  if(op_string=="r" || op_string=="ar" || op_string=="ra")
    op->needs(option_map["s"]);

  // excludes
  if(op_string=="a")
    op->excludes(option_map["s"]);

  if(op_string=="r")
    op->excludes(option_map["f"]);

  // Return the type of operation & callback verbosity
  std::tuple<OptOption, bool> option;
  auto& opt_option = std::get<OptOption>(option);
  auto& verbose_flag = std::get<bool>(option);

  if(*option_map.at("a"))
    opt_option = OptOption::agglomerate_only;
  else if(*option_map.at("r"))
    opt_option = OptOption::relax_only;
  else if(*option_map.at("ar"))
    opt_option = OptOption::agglomerate_then_relax;
  else if(*option_map.at("ra"))
    opt_option = OptOption::relax_then_agglomerate;

  if(*option_map.at("v"))
    verbose_flag = true;
  
  return option;
}


// output directory management
void manage_output_directory(const std::string outdir)
{
  bool flag;
  
  // create the output directory if it does not exist, erase mesh files
  if(!fs::exists(outdir)) {
    flag = fs::create_directory(outdir); assert(flag);
  }
  
  // check for the vtk subfolders
  bool has_vtk = false;
  for(const auto& entry : fs::directory_iterator(outdir)) {
    if(entry.is_directory() && entry.path().filename()=="vtk")
      has_vtk = true;
  }
  
  if(!has_vtk) {
    flag = fs::create_directory(outdir+"/vtk"); assert(flag);
  }
  
  // clear vtk files
  for (const auto& entry : fs::directory_iterator(outdir+"/vtk"))
    if (entry.is_regular_file() && entry.path().extension() == ".vtk")
      fs::remove(entry.path());
    
  return;
}
