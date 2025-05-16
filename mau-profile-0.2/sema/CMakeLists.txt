cmake_minimum_required(VERSION 3.4.0)

cmake_policy(SET CMP0042 OLD)	# Fix MACOSX_RPATH.
cmake_policy(SET CMP0048 NEW)	# Allow VERSION argument in project().
if (POLICY CMP0054)
	cmake_policy(SET CMP0054 NEW)	# No longer implicitly dereference variables.
endif()

set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)




if (NOT ${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64|AMD64")
	message(FATAL_ERROR "Target ${CMAKE_SYSTEM_PROCESSOR} not supported -- PTXSema works only on x86_64 architecture")
endif()


option(NO_DEBUG_TESTS "Create targets for CTest" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# Always use Release variant of C++ runtime.
	# We don't want to provide Debug variants of all dependencies. Some default
	# flags set by CMake must be tweaked.
	string(REPLACE "/MDd" "/MD" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	string(REPLACE "/D_DEBUG" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS OFF)
else()
	set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wextra -Wconversion -Wno-sign-conversion -Wno-unknown-pragmas ${CMAKE_CXX_FLAGS}")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT SANITIZE)
	# Do not allow unresolved symbols in shared library (default on linux)
	# unless sanitizer is used (sanity checks produce unresolved function calls)
	set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
endif()


project(ptxsema VERSION 0.1.0 LANGUAGES CXX C)

add_subdirectory(src)


# if (NOT NO_DEBUG_TESTS)
# 	enable_testing()
# 	add_subdirectory(tests)
# endif()
