# Sriramajayam

if(DOXYGEN_FOUND)

	message(STATUS ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
	
	# prepare doxygen configuration file
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
			     ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

	# add docs as custom target
	add_custom_target(docs ${DOXYGEN_EXECUTABLE}
			             ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

else()
	message(FATAL_ERROR "Cannot build documentation. Doxygen not found")
endif()
