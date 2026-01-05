// Sriramajayam

/** \file vm_io.h
 * \brief Defines helper function for mesh I/O
 * \author Ramsharan Rangarajan
 */

#pragma once

#include <pmp/surface_mesh.h>
#include <list>
#include <map>

namespace vm
{
  /** \brief Standard tag for storing face quality in a pmp::SurfaceMesh.
   *
   * This constant is used as the property name when by mesh i/o functions
   * to access and store face qualities.
   */
  inline const std::string Face_Quality_Tag = "face_quality";

  /** \brief Standard tag for storing vertex quality in a pmp::SurfaceMesh.
   *
   * This constant is used as the property name when by mesh i/o functions
   * to access and store vertex qualities.
   */
  inline const std::string Vertex_Quality_Tag = "vertex_quality";
    
  /** \brief Reads a polygonal mesh from a .OFF file into a SurfaceMesh.
   * 
   * This function parses a standard OFF file containing vertex coordinates
   * and face connectivity. 
   *
   * - Initializes face property `domain_id` to 0
   * - Initialize vertex property `interfac_id` to 0-1
   * 
   * \param[in] filename The path to the .OFF file to read.
   * \return The pmp::SurfaceMesh object that will be populated with
   *                  vertices and faces from the file.
   * \ingroup io
   */
  pmp::SurfaceMesh read_off(const std::string filename);

  /** \brief Writes a polygonal mesh to a .OFF file.
   * 
   * This function serializes a SurfaceMesh into the standard OFF format.
   * It writes vertex coordinates followed by face connectivity.
   * The output file can be read by read_off, or other standard OFF readers.
   * 
   * \param[in] mesh The SurfaceMesh object containing vertices and faces to write.
   * \param[in] filename The path to the .OFF file to create.
   * 
   * \note This function writes only the mesh geometry and connectivity.
   *       Face and vertex properties- `domain_id` and  `interface_id` are not written.
   * \ingroup io
   */
  void write_off(const pmp::SurfaceMesh& mesh,
		 const std::string filename);
  
  
  /** \brief Read a polygonal surface mesh from an ASCII VTK (.vtk) file.
   *
   * This function reads a VTK file in the legacy ASCII POLYDATA format and
   * populates a \c pmp::SurfaceMesh with vertices, polygonal faces, and
   * selected scalar data.
   *
   * The following data are read:
   * - Vertex coordinates from the \c POINTS section
   * - Polygonal faces from the \c POLYGONS section
   *
   * The following data are read if present; otherwise default properties are created
   * - Face scalar property \c domain_id (default = 1)
   * - Vertex scalar property \c interface_id (default = -1)
   *
   * The function assumes that the VTK file follows a standard, well-formed
   * legacy layout. In particular:
   * - The file is ASCII (not binary)
   * - Keywords \c POINTS, \c POLYGONS, \c domain_id, and
   *   \c interface_id appear on their own lines
   * - Vertex indices in the \c POLYGONS section are 0- or 1-based, and are sequential
   * - Only polygonal cells are present (no lines, strips, or mixed cell types)
   *
   * Other point or cell data are ignored.
   *
   * \param[in]  filename Path to the input .vtk file
   * \return Populated surface mesh populated
   *
   * \note
   * This reader is not a fully general VTK parser. Files that deviate from the
   * expected legacy ASCII POLYDATA format may not be read correctly.
   *
   * \pre \p filename has extension .vtk
   * \ingroup io
   */
  pmp::SurfaceMesh read_vtk(const std::string filename);

  /** \brief Write a polygonal surface mesh to an ASCII VTK file.
   *
   * Writes the given \c pmp::SurfaceMesh to file in legacy ASCII VTK
   * (\c POLYDATA) format. 
   *
   * The following data are written:
   * - Vertex coordinates (\c POINTS)
   * - Polygon connectivity (\c POLYGONS)
   * - Face scalar data:
   *   - \c domain_id (required)
   *   - \c face_quality (optional, if present)
   * - Vertex scalar data:
   *   - \c interface_id (required)
   *   - \c vertex_quality (optional, if present)
   *
   * Vertex indices in the output file are assumed to start from
   * either 0 or 1, depending on the internal indexing of the mesh.
   *
   * \note
   * This function writes only ASCII legacy VTK files and does not
   * support XML-based VTK formats (\c .vtp).
   *
   * \note
   * The mesh is assumed to have valid \c domain_id face properties
   * and \c interface_id vertex properties. Their absence will trigger
   * assertions.
   *
   * \param[in] mesh     Polygonal surface mesh to be written.
   * \param[in] filename Output filename (must have \c .vtk extension).
   *
   * \pre \c mesh.has_face_property("material_id") == true
   * \pre \c mesh.has_vertex_property("interface_id") == true
   * \ingroup io
   */
  void write_vtk(const pmp::SurfaceMesh& mesh,
		 const std::string filename);


   /** \brief Write the sorted face quality values of a mesh to a file.
   *
   * This function extracts the face quality scalar field stored in the mesh
   * under the standard tag \c vm::Face_Quality_Tag (="face_quality"), sorts the values in ascending
   * order, and writes them to a text file.
   *
   * The output file contains two columns:
   * - Column 1: 1-based index after sorting
   * - Column 2: Face quality value
   *
   * One entry is written per face.
   *
   * \param[in] mesh     Surface mesh containing face quality data.
   * \param[in] filename Path to the output file.
   *
   * \pre \c mesh.has_face_property(Face_Quality_Tag) == true
   * \ingroup io
   */
  void write_face_quality_vector(const pmp::SurfaceMesh &mesh,
				 const std::string filename);

  /** \brief Write the sorted vertex quality values of a mesh to a file.
   *
   * This function extracts the vertex quality scalar field stored in the mesh
   * under the standard tag \c Vertex_Quality_Tag (="vertex_quality"), sorts the values in ascending
   * order, and writes them to a text file.
   *
   * The output file contains two columns:
   * - Column 1: 1-based index after sorting
   * - Column 2: Vertex quality value
   *
   * One entry is written per vertex.
   *
   * \param[in] mesh     Surface mesh containing vertex quality data.
   * \param[in] filename Path to the output file.
   *
   * \pre \c mesh.has_vertex_property(Vertex_Quality_Tag) == true
   *
   * \ingroup io
   */
  void write_vertex_quality_vector(const pmp::SurfaceMesh &mesh,
				   const std::string filename);
}
