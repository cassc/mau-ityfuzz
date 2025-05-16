get_filename_component(PTXSEMA_INCLUDE_DIR ../include ABSOLUTE)


set(SOURCES_LIBS
	BasicBlock.cpp		BasicBlock.h
	Arith256.cpp		Arith256.h
	CompilerHelper.h	Common.h
	Instruction.cpp		Instruction.h
	Type.cpp			Type.h
	Optimizer.cpp		Optimizer.h
	Sanitizers.cpp      Sanitizers.h
	CodeGen.cpp 		CodeGen.h
	EEIModule.cpp		EEIModule.h
	WASMCompiler.cpp	WASMCompiler.h
	AFL.cpp 			AFL.h
	config.h
)

set(SOURCES ${SOURCES_LIBS} ${SOURCES_ENGINES})

source_group("" FILES ${SOURCES})

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fno-rtti -fvisibility=hidden")
	if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--exclude-libs,ALL") # Do not export symbols from dependies, mostly LLVM libs
	endif()
endif()

if(PTXSEMA_VERSION_MAJOR EQUAL 0)
	set(PTXSEMA_SOVERSION "0.${PTXSEMA_VERSION_MINOR}")
else()
	set(PTXSEMA_SOVERSION ${PTXSEMA_VERSION_MAJOR})
endif()


string(COMPARE EQUAL "${LLVM_ENABLE_ASSERTIONS}" "ON" LLVM_DEBUG)
configure_file(BuildInfo.h.in ${CMAKE_CURRENT_BINARY_DIR}/gen/BuildInfo.gen.h)

add_library(ptxsema ${SOURCES} gen/BuildInfo.gen.h)


# Explicit dependency on llvm to download LLVM header files.
add_dependencies(ptxsema LLVM::JIT)
get_target_property(LLVM_COMPILE_DEFINITIONS LLVM::JIT INTERFACE_COMPILE_DEFINITIONS)
if (LLVM_COMPILE_DEFINITIONS)
	target_compile_definitions(ptxsema PRIVATE ${LLVM_COMPILE_DEFINITIONS})
endif()
get_target_property(LLVM_INCLUDE_DIRECTORIES LLVM::JIT INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(ptxsema SYSTEM PRIVATE ${LLVM_INCLUDE_DIRECTORIES})
target_include_directories(ptxsema PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen)
target_include_directories(ptxsema PUBLIC ${PTXSEMA_INCLUDE_DIR})

# target_link_libraries(ptxsema PRIVATE  lld::lld LLVM::JIT)


set_target_properties(ptxsema PROPERTIES
	VERSION ${PTXSEMA_VERSION}
	SOVERSION ${PTXSEMA_SOVERSION}
	FOLDER "libs"
)
# target_compile_features(ptxsema PRIVATE cxx_std_17)

include(GNUInstallDirs)
install(TARGETS ptxsema
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY ${PTXSEMA_INCLUDE_DIR}/
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This section configures ptxsema-standalone -- a static library containing
# ptxsema and all its static dependencies.

# This function tries to get a list of all static library dependencies of
# a target. It probably does not handle all cases to be generic, but works for
# LLVM case.
function(get_link_libraries OUTPUT_LIST TARGET)
	get_target_property(IMPORTED ${TARGET} IMPORTED)
	list(APPEND VISITED_TARGETS ${TARGET})
	if (IMPORTED)
		get_target_property(LIBS ${TARGET} INTERFACE_LINK_LIBRARIES)
	else()
		get_target_property(LIBS ${TARGET} LINK_LIBRARIES)
	endif()
	set(LIB_FILES "")
	foreach(LIB ${LIBS})
		if (TARGET ${LIB})
            get_target_property(type ${LIB} TYPE)
            if(type STREQUAL INTERFACE_LIBRARY)
                continue()  # Skip interface libraries.
            endif()
			list(FIND VISITED_TARGETS ${LIB} VISITED)
			if (${VISITED} EQUAL -1)
				get_target_property(LIB_FILE ${LIB} LOCATION)
				get_link_libraries(LINK_LIB_FILES ${LIB})
				list(APPEND LIB_FILES ${LIB_FILE} ${LINK_LIB_FILES})
			endif()
		elseif (IS_ABSOLUTE ${LIB})
			# Here LIB is a full path to a (hopefuly static) library. We don't
			# want system libs (usually dynamic) here.
			list(APPEND LIB_FILES ${LIB})
		endif()
	endforeach()
	set(VISITED_TARGETS ${VISITED_TARGETS} PARENT_SCOPE)
	list(REMOVE_DUPLICATES LIB_FILES)
	set(${OUTPUT_LIST} ${LIB_FILES} PARENT_SCOPE)
endfunction()

# When building static lib add additional target ptxsema-standalone --
# an archive containing all LLVM dependencies in a single file.
get_target_property(_ptxsema_type ptxsema TYPE)
if (_ptxsema_type STREQUAL STATIC_LIBRARY)
	get_link_libraries(PTXSEMA_LINK_LIBRARIES ptxsema)
	set(PTXSEMA_STANDALONE_FILE ${CMAKE_STATIC_LIBRARY_PREFIX}ptxsema-standalone${CMAKE_STATIC_LIBRARY_SUFFIX})
	if (MSVC)
		set(_dummy_file ${CMAKE_CURRENT_BINARY_DIR}/ptxsema-standalone.c)
		if (NOT EXISTS ${_dummy_file})
			file(WRITE ${_dummy_file} "// ptxsema-standalone dummy file")
		endif()
		add_library(ptxsema-standalone STATIC EXCLUDE_FROM_ALL ${_dummy_file})
		add_dependencies(ptxsema-standalone ptxsema)
		string(REPLACE ";" " " FLAGS "${PTXSEMA_LINK_LIBRARIES}")
		set(FLAGS "${CMAKE_CFG_INTDIR}/ptxsema.lib ${FLAGS}")
		set_target_properties(ptxsema-standalone PROPERTIES STATIC_LIBRARY_FLAGS "${FLAGS}")
		install(TARGETS ptxsema-standalone
				RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
				LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
				ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
	elseif (APPLE)
		add_custom_command(OUTPUT ${PTXSEMA_STANDALONE_FILE}
						   COMMAND libtool -static -o ${PTXSEMA_STANDALONE_FILE} $<TARGET_FILE:ptxsema> ${PTXSEMA_LINK_LIBRARIES}
						   VERBATIM)
		add_custom_target(ptxsema-standalone DEPENDS ${PTXSEMA_STANDALONE_FILE})
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PTXSEMA_STANDALONE_FILE} DESTINATION ${CMAKE_INSTALL_LIBDIR} OPTIONAL)
	elseif (CMAKE_AR)
		# For platforms using ar a linker scripts is created and used to create
		# the standalone library.

		string(REPLACE ";" "\\naddlib " AR_SCRIPT ";${PTXSEMA_LINK_LIBRARIES}")
		set(AR_SCRIPT "create ${PTXSEMA_STANDALONE_FILE}\\naddlib $<TARGET_FILE:ptxsema>${AR_SCRIPT}\\nsave\\nend\\n")
		set(AR_SCRIPT_FILE ${CMAKE_STATIC_LIBRARY_PREFIX}ptxsema-standalone.ar-script)

		# Generate the linker script.
		add_custom_command(OUTPUT ${AR_SCRIPT_FILE}
				COMMAND printf ${AR_SCRIPT} > ${AR_SCRIPT_FILE}
				DEPENDS $<TARGET_FILE:ptxsema>
				VERBATIM)

		# Execute the script.
		add_custom_command(OUTPUT ${PTXSEMA_STANDALONE_FILE}
				COMMAND ${CMAKE_AR} -M < ${AR_SCRIPT_FILE}
				MAIN_DEPENDENCY ${AR_SCRIPT_FILE}
				VERBATIM)

		add_custom_target(ptxsema-standalone DEPENDS ${PTXSEMA_STANDALONE_FILE})

		# The "thin" library is also provided. It is smaller that the standalone one
		# but cannot be redistributed.
		set(PTXSEMA_STANDALONE_THIN_FILE ${CMAKE_STATIC_LIBRARY_PREFIX}ptxsema-standalone-thin${CMAKE_STATIC_LIBRARY_SUFFIX})

		add_custom_command(OUTPUT ${PTXSEMA_STANDALONE_THIN_FILE}
				COMMAND ${CMAKE_AR} cTq ${PTXSEMA_STANDALONE_THIN_FILE} $<TARGET_FILE:ptxsema> ${PTXSEMA_LINK_LIBRARIES})

		add_custom_target(ptxsema-standalone-thin ALL DEPENDS ${PTXSEMA_STANDALONE_THIN_FILE})
		# FIXME: Looks it will be better to create ptxsema-standalone as a library with costum steps instead of custom taget.
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PTXSEMA_STANDALONE_FILE} DESTINATION ${CMAKE_INSTALL_LIBDIR} OPTIONAL)
	endif()

	# Export the full path to ptxsema-standalone library to be used in tests.
	# TODO: It would be easier if ptxsema-standalone was a library target, but it
	#       works for now.
	set(PTXSEMA_STANDALONE_LIB ${CMAKE_CURRENT_BINARY_DIR}/${PTXSEMA_STANDALONE_FILE} PARENT_SCOPE)
endif()


# Compile standalone ptxsema
add_executable(standalone-ptxsema standalone.cpp ${SOURCES_LIBS})
add_dependencies(standalone-ptxsema LLVM::JIT)
if (LLVM_COMPILE_DEFINITIONS)
	target_compile_definitions(standalone-ptxsema PRIVATE ${LLVM_COMPILE_DEFINITIONS})
endif()
target_include_directories(standalone-ptxsema SYSTEM PRIVATE ${LLVM_INCLUDE_DIRECTORIES})
target_link_libraries(standalone-ptxsema PRIVATE LLVM::JIT)


# install(TARGETS standalone-ptxsema
# 		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# 		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
# 		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# install(TARGETS standalone-ptxsema
# 		RUNTIME DESTINATION ${CMAKE_BINARY_DIR})
# message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")

add_custom_command(TARGET standalone-ptxsema
  POST_BUILD 
  COMMAND mv ${PROJECT_BINARY_DIR}/src/standalone-ptxsema  ${CMAKE_BINARY_DIR}/ptxsema
  VERBATIM
)
