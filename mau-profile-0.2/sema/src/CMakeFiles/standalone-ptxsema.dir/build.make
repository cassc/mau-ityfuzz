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

# Include any dependencies generated for this target.
include sema/src/CMakeFiles/standalone-ptxsema.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include sema/src/CMakeFiles/standalone-ptxsema.dir/compiler_depend.make

# Include the progress variables for this target.
include sema/src/CMakeFiles/standalone-ptxsema.dir/progress.make

# Include the compile flags for this target's objects.
include sema/src/CMakeFiles/standalone-ptxsema.dir/flags.make

sema/src/CMakeFiles/standalone-ptxsema.dir/standalone.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/flags.make
sema/src/CMakeFiles/standalone-ptxsema.dir/standalone.cpp.o: sema/src/standalone.cpp
sema/src/CMakeFiles/standalone-ptxsema.dir/standalone.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mau/mau_profile-0.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object sema/src/CMakeFiles/standalone-ptxsema.dir/standalone.cpp.o"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT sema/src/CMakeFiles/standalone-ptxsema.dir/standalone.cpp.o -MF CMakeFiles/standalone-ptxsema.dir/standalone.cpp.o.d -o CMakeFiles/standalone-ptxsema.dir/standalone.cpp.o -c /mau/mau_profile-0.2/sema/src/standalone.cpp

sema/src/CMakeFiles/standalone-ptxsema.dir/standalone.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/standalone-ptxsema.dir/standalone.cpp.i"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mau/mau_profile-0.2/sema/src/standalone.cpp > CMakeFiles/standalone-ptxsema.dir/standalone.cpp.i

sema/src/CMakeFiles/standalone-ptxsema.dir/standalone.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/standalone-ptxsema.dir/standalone.cpp.s"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mau/mau_profile-0.2/sema/src/standalone.cpp -o CMakeFiles/standalone-ptxsema.dir/standalone.cpp.s

sema/src/CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/flags.make
sema/src/CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.o: sema/src/BasicBlock.cpp
sema/src/CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mau/mau_profile-0.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object sema/src/CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.o"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT sema/src/CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.o -MF CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.o.d -o CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.o -c /mau/mau_profile-0.2/sema/src/BasicBlock.cpp

sema/src/CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.i"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mau/mau_profile-0.2/sema/src/BasicBlock.cpp > CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.i

sema/src/CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.s"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mau/mau_profile-0.2/sema/src/BasicBlock.cpp -o CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.s

sema/src/CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/flags.make
sema/src/CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.o: sema/src/Arith256.cpp
sema/src/CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mau/mau_profile-0.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object sema/src/CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.o"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT sema/src/CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.o -MF CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.o.d -o CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.o -c /mau/mau_profile-0.2/sema/src/Arith256.cpp

sema/src/CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.i"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mau/mau_profile-0.2/sema/src/Arith256.cpp > CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.i

sema/src/CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.s"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mau/mau_profile-0.2/sema/src/Arith256.cpp -o CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.s

sema/src/CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/flags.make
sema/src/CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.o: sema/src/Instruction.cpp
sema/src/CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mau/mau_profile-0.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object sema/src/CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.o"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT sema/src/CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.o -MF CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.o.d -o CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.o -c /mau/mau_profile-0.2/sema/src/Instruction.cpp

sema/src/CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.i"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mau/mau_profile-0.2/sema/src/Instruction.cpp > CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.i

sema/src/CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.s"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mau/mau_profile-0.2/sema/src/Instruction.cpp -o CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.s

sema/src/CMakeFiles/standalone-ptxsema.dir/Type.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/flags.make
sema/src/CMakeFiles/standalone-ptxsema.dir/Type.cpp.o: sema/src/Type.cpp
sema/src/CMakeFiles/standalone-ptxsema.dir/Type.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mau/mau_profile-0.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object sema/src/CMakeFiles/standalone-ptxsema.dir/Type.cpp.o"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT sema/src/CMakeFiles/standalone-ptxsema.dir/Type.cpp.o -MF CMakeFiles/standalone-ptxsema.dir/Type.cpp.o.d -o CMakeFiles/standalone-ptxsema.dir/Type.cpp.o -c /mau/mau_profile-0.2/sema/src/Type.cpp

sema/src/CMakeFiles/standalone-ptxsema.dir/Type.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/standalone-ptxsema.dir/Type.cpp.i"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mau/mau_profile-0.2/sema/src/Type.cpp > CMakeFiles/standalone-ptxsema.dir/Type.cpp.i

sema/src/CMakeFiles/standalone-ptxsema.dir/Type.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/standalone-ptxsema.dir/Type.cpp.s"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mau/mau_profile-0.2/sema/src/Type.cpp -o CMakeFiles/standalone-ptxsema.dir/Type.cpp.s

sema/src/CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/flags.make
sema/src/CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.o: sema/src/Optimizer.cpp
sema/src/CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mau/mau_profile-0.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object sema/src/CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.o"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT sema/src/CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.o -MF CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.o.d -o CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.o -c /mau/mau_profile-0.2/sema/src/Optimizer.cpp

sema/src/CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.i"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mau/mau_profile-0.2/sema/src/Optimizer.cpp > CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.i

sema/src/CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.s"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mau/mau_profile-0.2/sema/src/Optimizer.cpp -o CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.s

sema/src/CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/flags.make
sema/src/CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.o: sema/src/Sanitizers.cpp
sema/src/CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mau/mau_profile-0.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object sema/src/CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.o"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT sema/src/CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.o -MF CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.o.d -o CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.o -c /mau/mau_profile-0.2/sema/src/Sanitizers.cpp

sema/src/CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.i"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mau/mau_profile-0.2/sema/src/Sanitizers.cpp > CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.i

sema/src/CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.s"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mau/mau_profile-0.2/sema/src/Sanitizers.cpp -o CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.s

sema/src/CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/flags.make
sema/src/CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.o: sema/src/CodeGen.cpp
sema/src/CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mau/mau_profile-0.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object sema/src/CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.o"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT sema/src/CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.o -MF CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.o.d -o CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.o -c /mau/mau_profile-0.2/sema/src/CodeGen.cpp

sema/src/CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.i"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mau/mau_profile-0.2/sema/src/CodeGen.cpp > CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.i

sema/src/CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.s"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mau/mau_profile-0.2/sema/src/CodeGen.cpp -o CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.s

sema/src/CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/flags.make
sema/src/CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.o: sema/src/EEIModule.cpp
sema/src/CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mau/mau_profile-0.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object sema/src/CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.o"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT sema/src/CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.o -MF CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.o.d -o CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.o -c /mau/mau_profile-0.2/sema/src/EEIModule.cpp

sema/src/CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.i"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mau/mau_profile-0.2/sema/src/EEIModule.cpp > CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.i

sema/src/CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.s"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mau/mau_profile-0.2/sema/src/EEIModule.cpp -o CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.s

sema/src/CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/flags.make
sema/src/CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.o: sema/src/WASMCompiler.cpp
sema/src/CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mau/mau_profile-0.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object sema/src/CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.o"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT sema/src/CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.o -MF CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.o.d -o CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.o -c /mau/mau_profile-0.2/sema/src/WASMCompiler.cpp

sema/src/CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.i"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mau/mau_profile-0.2/sema/src/WASMCompiler.cpp > CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.i

sema/src/CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.s"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mau/mau_profile-0.2/sema/src/WASMCompiler.cpp -o CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.s

sema/src/CMakeFiles/standalone-ptxsema.dir/AFL.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/flags.make
sema/src/CMakeFiles/standalone-ptxsema.dir/AFL.cpp.o: sema/src/AFL.cpp
sema/src/CMakeFiles/standalone-ptxsema.dir/AFL.cpp.o: sema/src/CMakeFiles/standalone-ptxsema.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mau/mau_profile-0.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object sema/src/CMakeFiles/standalone-ptxsema.dir/AFL.cpp.o"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT sema/src/CMakeFiles/standalone-ptxsema.dir/AFL.cpp.o -MF CMakeFiles/standalone-ptxsema.dir/AFL.cpp.o.d -o CMakeFiles/standalone-ptxsema.dir/AFL.cpp.o -c /mau/mau_profile-0.2/sema/src/AFL.cpp

sema/src/CMakeFiles/standalone-ptxsema.dir/AFL.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/standalone-ptxsema.dir/AFL.cpp.i"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mau/mau_profile-0.2/sema/src/AFL.cpp > CMakeFiles/standalone-ptxsema.dir/AFL.cpp.i

sema/src/CMakeFiles/standalone-ptxsema.dir/AFL.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/standalone-ptxsema.dir/AFL.cpp.s"
	cd /mau/mau_profile-0.2/sema/src && /usr/bin/g++-7 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mau/mau_profile-0.2/sema/src/AFL.cpp -o CMakeFiles/standalone-ptxsema.dir/AFL.cpp.s

# Object files for target standalone-ptxsema
standalone__ptxsema_OBJECTS = \
"CMakeFiles/standalone-ptxsema.dir/standalone.cpp.o" \
"CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.o" \
"CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.o" \
"CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.o" \
"CMakeFiles/standalone-ptxsema.dir/Type.cpp.o" \
"CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.o" \
"CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.o" \
"CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.o" \
"CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.o" \
"CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.o" \
"CMakeFiles/standalone-ptxsema.dir/AFL.cpp.o"

# External object files for target standalone-ptxsema
standalone__ptxsema_EXTERNAL_OBJECTS =

sema/src/standalone-ptxsema: sema/src/CMakeFiles/standalone-ptxsema.dir/standalone.cpp.o
sema/src/standalone-ptxsema: sema/src/CMakeFiles/standalone-ptxsema.dir/BasicBlock.cpp.o
sema/src/standalone-ptxsema: sema/src/CMakeFiles/standalone-ptxsema.dir/Arith256.cpp.o
sema/src/standalone-ptxsema: sema/src/CMakeFiles/standalone-ptxsema.dir/Instruction.cpp.o
sema/src/standalone-ptxsema: sema/src/CMakeFiles/standalone-ptxsema.dir/Type.cpp.o
sema/src/standalone-ptxsema: sema/src/CMakeFiles/standalone-ptxsema.dir/Optimizer.cpp.o
sema/src/standalone-ptxsema: sema/src/CMakeFiles/standalone-ptxsema.dir/Sanitizers.cpp.o
sema/src/standalone-ptxsema: sema/src/CMakeFiles/standalone-ptxsema.dir/CodeGen.cpp.o
sema/src/standalone-ptxsema: sema/src/CMakeFiles/standalone-ptxsema.dir/EEIModule.cpp.o
sema/src/standalone-ptxsema: sema/src/CMakeFiles/standalone-ptxsema.dir/WASMCompiler.cpp.o
sema/src/standalone-ptxsema: sema/src/CMakeFiles/standalone-ptxsema.dir/AFL.cpp.o
sema/src/standalone-ptxsema: sema/src/CMakeFiles/standalone-ptxsema.dir/build.make
sema/src/standalone-ptxsema: deps/lib/libLLVMNVPTXCodeGen.a
sema/src/standalone-ptxsema: deps/lib/libLLVMNVPTXDesc.a
sema/src/standalone-ptxsema: deps/lib/libLLVMNVPTXInfo.a
sema/src/standalone-ptxsema: deps/lib/libLLVMMCDisassembler.a
sema/src/standalone-ptxsema: deps/lib/libLLVMCFGuard.a
sema/src/standalone-ptxsema: deps/lib/libLLVMGlobalISel.a
sema/src/standalone-ptxsema: deps/lib/libLLVMSelectionDAG.a
sema/src/standalone-ptxsema: deps/lib/libLLVMAsmPrinter.a
sema/src/standalone-ptxsema: deps/lib/libLLVMDebugInfoDWARF.a
sema/src/standalone-ptxsema: deps/lib/libLLVMMCJIT.a
sema/src/standalone-ptxsema: deps/lib/libLLVMipo.a
sema/src/standalone-ptxsema: deps/lib/libLLVMFrontendOpenMP.a
sema/src/standalone-ptxsema: deps/lib/libLLVMExecutionEngine.a
sema/src/standalone-ptxsema: deps/lib/libLLVMRuntimeDyld.a
sema/src/standalone-ptxsema: deps/lib/libLLVMLTO.a
sema/src/standalone-ptxsema: deps/lib/libLLVMPasses.a
sema/src/standalone-ptxsema: deps/lib/libLLVMObjCARCOpts.a
sema/src/standalone-ptxsema: deps/lib/libLLVMInstrumentation.a
sema/src/standalone-ptxsema: deps/lib/libLLVMVectorize.a
sema/src/standalone-ptxsema: deps/lib/libLLVMLinker.a
sema/src/standalone-ptxsema: deps/lib/libLLVMIRReader.a
sema/src/standalone-ptxsema: deps/lib/libLLVMAsmParser.a
sema/src/standalone-ptxsema: deps/lib/libLLVMCodeGen.a
sema/src/standalone-ptxsema: deps/lib/libLLVMTarget.a
sema/src/standalone-ptxsema: deps/lib/libLLVMScalarOpts.a
sema/src/standalone-ptxsema: deps/lib/libLLVMInstCombine.a
sema/src/standalone-ptxsema: deps/lib/libLLVMBitWriter.a
sema/src/standalone-ptxsema: deps/lib/libLLVMAggressiveInstCombine.a
sema/src/standalone-ptxsema: deps/lib/libLLVMTransformUtils.a
sema/src/standalone-ptxsema: deps/lib/libLLVMAnalysis.a
sema/src/standalone-ptxsema: deps/lib/libLLVMProfileData.a
sema/src/standalone-ptxsema: deps/lib/libLLVMObject.a
sema/src/standalone-ptxsema: deps/lib/libLLVMTextAPI.a
sema/src/standalone-ptxsema: deps/lib/libLLVMMCParser.a
sema/src/standalone-ptxsema: deps/lib/libLLVMMC.a
sema/src/standalone-ptxsema: deps/lib/libLLVMDebugInfoCodeView.a
sema/src/standalone-ptxsema: deps/lib/libLLVMDebugInfoMSF.a
sema/src/standalone-ptxsema: deps/lib/libLLVMBitReader.a
sema/src/standalone-ptxsema: deps/lib/libLLVMCore.a
sema/src/standalone-ptxsema: deps/lib/libLLVMRemarks.a
sema/src/standalone-ptxsema: deps/lib/libLLVMBitstreamReader.a
sema/src/standalone-ptxsema: deps/lib/libLLVMBinaryFormat.a
sema/src/standalone-ptxsema: deps/lib/libLLVMSupport.a
sema/src/standalone-ptxsema: deps/lib/libLLVMDemangle.a
sema/src/standalone-ptxsema: deps/lib/libLLVMOption.a
sema/src/standalone-ptxsema: sema/src/CMakeFiles/standalone-ptxsema.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mau/mau_profile-0.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Linking CXX executable standalone-ptxsema"
	cd /mau/mau_profile-0.2/sema/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/standalone-ptxsema.dir/link.txt --verbose=$(VERBOSE)
	cd /mau/mau_profile-0.2/sema/src && mv /mau/mau_profile-0.2/sema/src/standalone-ptxsema /mau/mau_profile-0.2/ptxsema

# Rule to build all files generated by this target.
sema/src/CMakeFiles/standalone-ptxsema.dir/build: sema/src/standalone-ptxsema
.PHONY : sema/src/CMakeFiles/standalone-ptxsema.dir/build

sema/src/CMakeFiles/standalone-ptxsema.dir/clean:
	cd /mau/mau_profile-0.2/sema/src && $(CMAKE_COMMAND) -P CMakeFiles/standalone-ptxsema.dir/cmake_clean.cmake
.PHONY : sema/src/CMakeFiles/standalone-ptxsema.dir/clean

sema/src/CMakeFiles/standalone-ptxsema.dir/depend:
	cd /mau/mau_profile-0.2 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mau/mau_profile-0.2 /mau/mau_profile-0.2/sema/src /mau/mau_profile-0.2 /mau/mau_profile-0.2/sema/src /mau/mau_profile-0.2/sema/src/CMakeFiles/standalone-ptxsema.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : sema/src/CMakeFiles/standalone-ptxsema.dir/depend

