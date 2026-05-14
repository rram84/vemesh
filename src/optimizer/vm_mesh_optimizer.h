// Sriramajayam

/** \file vm_mesh_optimizer.h
 * \brief Defines main class vm::MeshOptimizer that performs face agglomeration and vertex relaxation operations
 * \author Ramsharan Rangarajan
 */

#pragma once

#include <pmp/surface_mesh.h>
#include <vm_quality_evaluator.h>
#include <string>
#include <tuple>
#include <set>
#include <map>
#include <random>
#include <optional>

namespace vm
{
  class MeshOptimizer;

  //! \brief Struct encapsulating the progress of mesh improvement iterations
  //! The members are context specific- referring either to agglomeration or vertex relaxation iterations.
  struct ProgressInfo {
    int    idx;           //!< Index of agglomerated face, or relaxed vertex
    int    num_target;    //!< Number of faces/vertices considered for improvement
    int    num_completed; //!< Cumulative number of faces agglomerated thus far, or vertices relaxed thus far
    double quality;       //!< Revised quality of the agglomerated face or relaxed vertex
  };

  /**
   * \brief Progress callback invoked during mesh optimization.
   *
   * The callback is called 
   * - after a face is agglomerated, or
   * - a vertex is relaxed.
   *
   * It allows the user to inspect the current state of the mesh, record
   * intermediate results (e.g., saving the mesh to file), or terminate the mesh optimization early.
   *
   * \param info       Struct containing information about the last executed mesh optimization operation
   * \param mesh       Read-only reference to the current mesh state.
   * \param optimizer  Reference to the optimizer performing the operation
   *
   * \return
   * - `true`  to continue the operation
   * - `false` to abort the operation immediately
   * 
   * \note The returned boolean only terminates further optimization; it does not revert a completed operation.
   *
   * \note
   * The callback cannot be used to modify the mesh or call mutating member functions of `MeshOptimizer`. 
   *
   * \note
   * The callback is called after each face/vertex optimization operation. It should therefore be lightweight.
   */
  using ProgressCallback = std::function<bool(const ProgressInfo& info, const pmp::SurfaceMesh &mesh, const MeshOptimizer &optimizer)>;

  /** \brief Class implementing the main functionality of the library: vertex relaxation and element agglomeration
   *
   * ---
   *
   * **The Mesh**: \n
   * The class maintains a persistent copy of the underlying mesh. \n
   * The mesh is represented as an instance of pmp::SurfaceMesh. No other types are supported.\n
   * Face vertices in the mesh are assumed to be ordered in a counter-clockwise sense.
   * The mesh is required to have the following  properties:
   *
   * | Property  | Attribute | Data type | Notes    |
   * |-----------|-----------|-----------|----------|
   * | `interface_id`  | Vertex property | int      | Identifier of interface on which a vertex lies; -1 for vertices not lying on an interface |
   * | `domain_id`     | Face property   | int      | Identifier of domain to which a face belongs |
   *
   * The class provides immutable external access to the mesh. \n
   * The assignment of domain and interface identifiers to faces and vertices is assumed at construction. \n
   * When requested, face and vertex qualities evaluated  by the class are stored as face and vertex properties in the mesh.
   * The property tags for these are specified by the user at the time of evaluation.
   * 
   * ---
   *
   * **Element agglomeration**: \n
   * One of the two main functionalities provided by the class is element agglomeration. \n
   * Element agglomeration attempts to merge faces with their neighbors to improve quality, as defined by a user-specified metric. \n
   * The operation changes the element count and connectivity of the mesh, but leaves the vertex set unchanged.\n
   * Given a face quality metric \f$Q\f$, the class enables merging a face \f$f\f$ with a neighbor \f$n_f\f$ to create a new face \f$g\f$, provided that:
   * - the set of *agglomerable* neighbors \f${\cal A}(f)\f$ is not empty. 
   * - \f$Q(g)>\epsilon\f$ for a given lower bound \f$\epsilon\f$ for face quality, and
   * - \f$Q(g)\geq \eta Q(f)\f$, i.e., the quality of \f$g\f$ is improved sufficiently over \f$f\f$.
   * The quality of the agglomerated neighbor \f$n_f\f$ may deteriorate as a result. This is essential- it is  precisely why improvement is possible in the first place.
   * Faces of better quality are compromised to improve the qualities of poorer faces.
   *
   * The class provides three overloaded methods for element agglomeration:
   * - \ref agglomerate(const pmp::Face&, const QualityEvaluator&, double, double): \n
   * Attempts to agglomerate a specified face. It provides the most granular control.
   *
   * - \ref agglomerate(const std::set<pmp::Face>&, const QualityEvaluator& QE, double, double, const ProgressCallback &): \n
   * Attempts agglomerating faces in a specified subset of faces, starting from the poorest one first.
   *
   * - \ref  agglomerate(const QualityEvaluator&, double, double, const ProgressCallback &): \n
   * First determines the subset of faces to be considered for agglomeration by performing a mesh-wide search
   * to tag faces with quality below the specified threshold \f$\epsilon\f$.
   * Then, attempts agglomerating them, starting from the poorest one first.
   *
   * In general, an attempt to agglomerate a face may not be successful. This may be because there isn't an agglomerable neighbor, or 
   * the aglomerated face's quality is not sufficiently better. 
   * 
   * ---
   *
   * **Vertex relaxation**: \n
   * The second functionality provided by the class is vertex relaxation. \n
   * Only vertices not lying on an interface can be perturbed.
   * The operation perturbs a vertex to a new location to improve the qualities of incident faces. \n
   * Specifically, let \f$f_1, \ldots, f_n\f$ be the set of faces incident at a vertex \f$v\f$. \n
   * Denote the quality of an incident face \f$f\f$ with \f$v\f$ positioned at \f$x\f$ as \f$Q(f;x)\f$. \n
   * The class identifies a new location \f$y\f$ for \f$v\f$ such that: 
   * \f[\min_{1\leq i\leq n}Q(f_i,y) > \min_{1\leq i\leq n}Q(f_i,x)\f] 
   * It is useful to interpret \f$Q(v,x) = \min_{1\leq i\leq n}Q(f_i,x)\f$ as the quality of the vertex \f$v\f$. \n
   * Candidate locations for the new vertex position are generated by random sampling. \n
   * The number of samples to generate is specified by the user. Generally, prefer larger number of samples for a poorer mesh.
   * In addition, the midpoint of each edge incident at the vertex is also sampled.
   *
   * The class provides three overloaded methods for vertex relaxation:
   * - \ref relax(const pmp::Vertex&, const QualityEvaluator&, int): \n
   * Provides the most granular control. It attempts to relocate a specified vertex to a new location.
   *
   * - \ref relax(const std::set<pmp::Vertex>&, const QualityEvaluator&, int, const ProgressCallback &): \n
   * Attempts to relax vertices in a specified set, starting from the vertex with the poorest quality.  
   *
   * - \ref relax(const QualityEvaluator&, double, int, const ProgressCallback &): \n
   * First determines the subset of vertices to be considered for relaxation by performing a mesh-wide search to tag non-interface and non-boundary vertices
   * with quality below the specified threshold \f$\epsilon\f$.Then, attempts relaxing them, starting from the poorest one first.
   * 
   * ---
   *
   * **Evaluating face/vertex qualities**: \n
   * The class computes face and vertex qualities on the fly. This is essential because
   * stored qualities are frequently invalidated by agglomeration and relaxation operations. 
   * Moreover, the user is free to specify different quality during a mesh optimization operation.
   * 
   * For convenience, the class provides methods `evaluate_face_qualities` and `evaluate_vertex_qualities` to evaluate vertex qualities.
   * The evaluation of vertex qualities is based on an existing evaluation of face qualities.
   * 
   * ---
   * \ingroup optimizer
   */
  class MeshOptimizer
  {
  public:
    //! \brief Constructor, with mesh copied from a given instance
    //!
    //! Assumes that the mesh has the vertex property `interface_id` and face property `domain_id` defined,
    //! and that all faces list their vertices in counter-clockwise order.
    //! \param[in] in_mesh Input mesh, copied
    MeshOptimizer(const pmp::SurfaceMesh& in_mesh);
    
    //! \brief Destructor
    ~MeshOptimizer() = default;

    //! Disable copy and assignment
    MeshOptimizer(const MeshOptimizer&) = delete;
    MeshOptimizer& operator=(const MeshOptimizer&) = delete;

    //! \brief Immutable access to the mesh
    //! \return Const reference to the mesh
    inline const pmp::SurfaceMesh& get_mesh() const
    { return mesh; }
    
    //! \brief Agglomerate a face with a feasible neighbor
    //! \param[in] f The face to agglomerate. 
    //! \param[in] QE Reference to an instance of QualityEvaluator, used to evaluate face qualities
    //! \param[in] qfactor Lower bound for factor of improvement in face quality, assumed to be greater than 1.
    //! \return A triplet `result` such that:
    //! - `std::get<0>(result)`: true if the face was agglomerated, and false otherwise
    //! - `std::get<1>(result)`: quality of the agglomerated face in case of success, of the existing face otherwise
    //! - `std::get<2>(result)`: the new agglomerated face in case of success, and `f' otherwise
    std::tuple<bool, double, pmp::Face> agglomerate(const pmp::Face& f,
						    const QualityEvaluator& QE,
						    double qfactor);

    //! \brief Agglomerate a given set of faces
    //! \param[in] subset Given subset of faces in the mesh.
    //! \param[in] QE Reference to an instance of QualityEvaluator, used to evaluate face qualities
    //! \param[in] qfactor Lower bound for factor of improvement in face quality,
    //!                    assumed to be greater than 1.
    //! \param[in] callback Callback function invoked after each successful agglomeration attempt
    //! \return Number of aglomerated faces. Can help decide if another iteration of agglomeration is warranted
    int agglomerate(const std::set<pmp::Face>& subset,
		    const QualityEvaluator& QE,
		    double qfactor,
		    const ProgressCallback &callback=nullptr);

    //! \brief Agglomerate faces in a mesh
    //! Attempts merging all faces satisfying \f$Q(f)\leq \epsilon\f$ for the given quality threshold \f$\epsilon\f$.
    //! \param[in] QE Reference to an instance of QualityEvaluator, used to evaluate face qualities
    //! \param[in] qmin Acceptable positive lower bound for quality
    //! \param[in] qfactor Lower bound for factor of improvement in face quality, assumed to be greater than 1.
    //! \param[in] callback Callback function invoked after each successful agglomeration attempt
    //! \return Number of agglomerated faces.  Can help decide if another iteration of agglomeration is warranted
    int agglomerate(const QualityEvaluator& QE,
		    double qmin,
		    double qfactor,
		    const ProgressCallback &callback=nullptr);

    //! \brief Relaxes a vertex to a more favorable position
    //! \param[in] vertex Vertex to consider relaxing
    //! \param[in] num_samples Number of sample points to generate per strategy:
    //!                        (i) within a bounding rectangle enclosing the faces incident at the vertex.
    //!                        (ii) as convex combinations of the vertex and its 1-ring of vertices
    //! \param[in] QE Reference to an instance of QualityEvaluator, used to evaluate face qualities incident at a vertex
    //! \param[in] seed Optional RNG seed. When supplied, the internal RNG is re-seeded with this value at the start of the call,
    //!                 making the result reproducible across invocations. Omit (the default) for nondeterministic behavior.
    //!
    //! \return A pair `result`. The boolean `result->first` indicates if the relaxation was successful.
    //! The double `result->second` returns the quality of the vertex at its new location if the relaxation was successful, and
    //! at its existing location otherwise.
    //! \note A total of 2*num_samples sample points are generated. 
    //! In general, only a (small) fraction of these sample points will be *feasible*.
    std::pair<bool,double> relax(const pmp::Vertex& vertex,
				 const QualityEvaluator& QE,
				 int num_samples,
				 std::optional<unsigned int> seed = std::nullopt);

    //! \brief Relaxes a specified subset of vertices to more favorable positions.
    //! \param[in] subset Set of vertices to consider relaxing
    //! \param[in] num_samples Number of sample points to generate for *each* vertex, per strategy:
    //                         (i) within a bounding rectangle enclosing the faces incident at the vertex.
    //!                        (ii) as convex combinations of the vertex and its 1-ring of vertices
    //! \param[in] QE Reference to an instance of QualityEvaluator, used to evaluate face qualities incident at a vertex
    //! \param[in] callback Callback function invoked after each successful vertex relaxation
    //! \param[in] seed Optional RNG seed. When supplied, the internal RNG is re-seeded with this value at the start of the call,
    //!                 making the result reproducible across invocations. Omit (the default) for nondeterministic behavior.
    //!
    //! \return Number of relaxed vertices. Can help decide if another iteration of vertex relaxations is warranted
    //! \note A total of 2*num_samples sample points are generated per vertex. 
    //! In general, only a (small) fraction of these sample points will be *feasible*.
    int relax(const std::set<pmp::Vertex>& subset,
	      const QualityEvaluator& QE,
	      int num_samples,
	      const ProgressCallback &callback=nullptr,
	      std::optional<unsigned int> seed = std::nullopt);

    //! \brief Relaxes vertices of poor quality to more favorable positions.
    //! Attempts relaxing all vertices with quality less than a given tolerance.
    //! \param[in] num_samples Number of sample points to generate for *each* vertex, per strategy:
    //!                         (i) within a bounding rectangle enclosing the faces incident at the vertex.
    //!                        (ii) as convex combinations of the vertex and its 1-ring of vertices
    //! \param[in] QE Reference to an instance of QualityEvaluator, used to evaluate face qualities incident at a vertex
    //! \param[in] qmin Positive lower bound for quality used to identify vertices to relax
    //! \param[in] callback Callback function invoked after each successful vertex relaxation
    //! \param[in] seed Optional RNG seed. When supplied, the internal RNG is re-seeded with this value at the start of the call,
    //!                 making the result reproducible across invocations. Omit (the default) for nondeterministic behavior.
    //!
    //! \return Number of relaxed vertices. Can help decide if another iteration of vertex relaxations is warranted
    //! \note A total of 2*num_samples sample points are generated per vertex. 
    //! In general, only a (small) fraction of these sample points will be *feasible*.
    int relax(const QualityEvaluator& QE,
	      double qmin,
	      int num_samples,
	      const ProgressCallback &callback=nullptr,
	      std::optional<unsigned int> seed = std::nullopt);
    
    //! \brief Evaluates the qualities of all faces in the mesh and saves it as a face property in the mesh
    //! \param[in] QE Reference to an instance of QualityEvaluator, used to evaluate face qualities
    //! \param[in] face_quality_tag Tag under which to save evaluated qualities. If the property already exists, values are overwritten.
    void evaluate_face_qualities(const QualityEvaluator& QE, std::string face_quality_tag);

    //! \brief Evaluates the qualities of all vertices in the mesh and saves it as a vertex property in the mesh
    //! Specifically, the quality at a vertex is the minimum among the qualities of faces incident at it. The latter
    //! are accessed as a face property of the mesh using the given property tag.
    //! \param[in] face_quality_tag Tag of an *existing* face property, storing qualities of all faces in the mesh
    //! \param[in] vertex_quality_tag Tag under which vertex qualities are saved. If the property already exists, values are overwritten.
    void evaluate_vertex_qualities(std::string face_quality_tag, std::string vertex_quality_tag);
    
  protected:
    //! \brief Helper method to determine agglomerable neighbors of a face along a given halfedge
    //! A halfedge is consider agglomerable if the adjacent faces can be merged. For this:
    //! - The halfedge should be valid
    //! - The halfedge should not lie on the boundary
    //! - The two faces adjacent to the half edge should be valid
    //! - Both faces adjacent to the half edge should have the same domain id
    //! - Both vertices of the halfedge should have valence>2
    //! \param[in] h Halfedge.
    //! \return True if agglomerable, and false otherwise
    bool is_agglomerable(const pmp::Halfedge& h) const;
			 
    //! \brief Helper method to identify the optimal agglomerable neighbor for merging a given face
    //! \param[in] face Face to consider
    //! \param[in] QE QualityEvaluator instance used to compute face qualities
    std::tuple<bool, double, pmp::Halfedge> find_halfedge_for_face_merge(const pmp::Face& face,
									 const QualityEvaluator& QE) const;
    
    //! \brief Helper method to execute a merge faces incident at a given halfedge
    //! \param[in] halfedge Halfedge at which to merge faces
    //! \return New agglomerated face
    pmp::Face merge_neighbors(const pmp::Halfedge& halfedge);
    
    //! \brief Helper method to generate feasible positions to perturb a given vertex to
    //! \param[in] vertex Vertex to perturb
    //! \param[in] num_samples Number of random samples to generate
    //!                        (i) within the bounding box of faces incident at the vertex, and 
    //!                        (ii) as convex combinations of vertices in the 1-ring
    //! \return Vector of pairs of (x,y) coordinates of feasible points. Possibly empty.
    std::vector<std::pair<double,double>>
      compute_feasible_vertex_positions(const pmp::Vertex& vertex, const int num_samples) const; 

    //! \brief Helper method to compute new location for a vertex
    //! \param[in] vertex Vertex to perturb
    //! \param[in] num_samples Number of random samples to generate
    //!                        (i) within the bounding box of faces incident at the vertex, and 
    //!                        (ii) as convex combinations of vertices in the 1-ring
    //! \param[in] QE Instance of QualityEvaluator used to compute face qualities
    //! \return Triplet result, such that: \n
    //! `std::get<bool>(result)` indicates whether an improved location was found
    //! `std::get<pmp::Point>` equals the new location in case of success, and the current location otherwise \n
    //! std::get<double>` equals the new quality at the vertex in case of success, and the current quality otherwise 
    std::tuple<bool, pmp::Point, double>
      compute_improved_vertex_position(const pmp::Vertex      &vertex,
				       const int              num_samples,
				       const QualityEvaluator& QE);

    
    pmp::SurfaceMesh mesh; //!< Persistent mesh

  private:
    //! \brief Persistent RNG used for vertex-relaxation sampling.
    //!
    //! Seeded nondeterministically by the constructor. Public relax methods
    //! re-seed it on entry when their optional `seed` argument is supplied.
    //! Marked `mutable` so it can advance from inside the `const` helper
    //! `compute_feasible_vertex_positions`.
    mutable std::mt19937 rng;
  };
}
