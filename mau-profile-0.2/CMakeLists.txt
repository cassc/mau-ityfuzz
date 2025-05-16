cmake_minimum_required(VERSION 3.20.0)

cmake_policy(SET CMP0042 OLD)	# Fix MACOSX_RPATH.
cmake_policy(SET CMP0048 NEW)	# Allow VERSION argument in project().
if (POLICY CMP0054)
	cmake_policy(SET CMP0054 NEW)	# No longer implicitly dereference variables.
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# setting LLVM framework FIXME: upgrade to LLVM-16 from LLVM-13
include(ProjectLLVM)
configure_llvm_project()

add_subdirectory(sema)
add_subdirectory(runtime)
add_subdirectory(runner)
add_subdirectory(test)
