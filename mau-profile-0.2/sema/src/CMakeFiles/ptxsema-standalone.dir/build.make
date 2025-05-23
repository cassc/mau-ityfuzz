# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mau/mau_profile-0.2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mau/mau_profile-0.2

# Utility rule file for ptxsema-standalone.

# Include any custom commands dependencies for this target.
include sema/src/CMakeFiles/ptxsema-standalone.dir/compiler_depend.make

# Include the progress variables for this target.
include sema/src/CMakeFiles/ptxsema-standalone.dir/progress.make

sema/src/CMakeFiles/ptxsema-standalone: sema/src/libptxsema-standalone.a

sema/src/libptxsema-standalone.a: sema/src/libptxsema-standalone.ar-script
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/mau/mau_profile-0.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating libptxsema-standalone.a"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/ar -M < libptxsema-standalone.ar-script

sema/src/libptxsema-standalone.ar-script: sema/src/libptxsema.a
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/mau/mau_profile-0.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Generating libptxsema-standalone.ar-script"
	cd /mau/mau_profile-0.2/sema/src && printf "create libptxsema-standalone.a\\naddlib /mau/mau_profile-0.2/sema/src/libptxsema.a\\naddlib \\nsave\\nend\\n" > libptxsema-standalone.ar-script

ptxsema-standalone: sema/src/CMakeFiles/ptxsema-standalone
ptxsema-standalone: sema/src/libptxsema-standalone.a
ptxsema-standalone: sema/src/libptxsema-standalone.ar-script
ptxsema-standalone: sema/src/CMakeFiles/ptxsema-standalone.dir/build.make
.PHONY : ptxsema-standalone

# Rule to build all files generated by this target.
sema/src/CMakeFiles/ptxsema-standalone.dir/build: ptxsema-standalone
.PHONY : sema/src/CMakeFiles/ptxsema-standalone.dir/build

sema/src/CMakeFiles/ptxsema-standalone.dir/clean:
	cd /mau/mau_profile-0.2/sema/src && $(CMAKE_COMMAND) -P CMakeFiles/ptxsema-standalone.dir/cmake_clean.cmake
.PHONY : sema/src/CMakeFiles/ptxsema-standalone.dir/clean

sema/src/CMakeFiles/ptxsema-standalone.dir/depend:
	cd /mau/mau_profile-0.2 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mau/mau_profile-0.2 /mau/mau_profile-0.2/sema/src /mau/mau_profile-0.2 /mau/mau_profile-0.2/sema/src /mau/mau_profile-0.2/sema/src/CMakeFiles/ptxsema-standalone.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : sema/src/CMakeFiles/ptxsema-standalone.dir/depend

