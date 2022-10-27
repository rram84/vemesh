# Sriramajayam

include(ExternalProject)

function(vemesh_download_cli11 build_path)
	ExternalProject_Add(
	CLI11
	GIT_REPOSITORY https://github.com/CLIUtils/CLI11
	GIT_TAG        v2.2.0
	PREFIX	       ${build_path}
#	INSTALL_COMMAND ""
	CMAKE_ARGS
			-DCLI11_SINGLE_FILE=ON
			-DCLI11_BUILD_TESTS=OFF
			-DCLI11_BUILD_EXAMPLES=OFF
			-DCLI11_BUILD_DOCS=OFF
			-DCMAKE_BUILD_TYPE=Release
	)
endfunction()


function(vemesh_download_pmp build_path)
	ExternalProject_Add(
	pmp-library
	GIT_REPOSITORY	https://github.com/pmp-library/pmp-library
	GIT_TAG		2.0.0
	PREFIX          ${build_path}
	INSTALL_COMMAND ""
	CMAKE_ARGS
			-DPMP_BUILD_VIS=OFF
			-DPMP_BUILD_EXAMPLES=OFF
			-DPMP_BUILD_TESTS=OFF
			-DPMP_BUILD_DOCS=OFF
			-DCMAKE_BUILD_TYPE=Release
	)
endfunction()