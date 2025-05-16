
# Configures LLVM dependency
#
# This function handles everything needed to setup LLVM project.
# By default it downloads and builds LLVM from source.
#
# Creates a target representing all required LLVM libraries and include path.
function(configure_llvm_project)
    # Generated with `llvm-config --libs mcjit ipo all-targets lto option`
    set(LIBS
        # LLVMXCoreDisassembler 
        # LLVMXCoreCodeGen 
        # LLVMXCoreDesc LLVMXCoreInfo
        # LLVMX86Disassembler LLVMX86AsmParser LLVMX86CodeGen LLVMX86Desc LLVMX86Utils LLVMX86Info 
        # LLVMWebAssemblyDisassembler LLVMWebAssemblyCodeGen 
        # LLVMWebAssemblyDesc LLVMWebAssemblyAsmParser LLVMWebAssemblyInfo 
        # LLVMSystemZDisassembler LLVMSystemZCodeGen LLVMSystemZAsmParser 
        # LLVMSystemZDesc LLVMSystemZInfo LLVMSparcDisassembler LLVMSparcCodeGen 
        # LLVMSparcAsmParser LLVMSparcDesc LLVMSparcInfo 
        # LLVMRISCVDisassembler LLVMRISCVCodeGen LLVMRISCVAsmParser LLVMRISCVDesc LLVMRISCVUtils LLVMRISCVInfo 
        # LLVMPowerPCDisassembler LLVMPowerPCCodeGen LLVMPowerPCAsmParser LLVMPowerPCDesc LLVMPowerPCInfo 
        LLVMNVPTXCodeGen LLVMNVPTXDesc LLVMNVPTXInfo
        # LLVMMSP430Disassembler LLVMMSP430CodeGen LLVMMSP430AsmParser LLVMMSP430Desc LLVMMSP430Info 
        # LLVMMipsDisassembler LLVMMipsCodeGen LLVMMipsAsmParser LLVMMipsDesc LLVMMipsInfo 
        # LLVMLanaiDisassembler LLVMLanaiCodeGen LLVMLanaiAsmParser LLVMLanaiDesc LLVMLanaiInfo 
        # LLVMHexagonDisassembler LLVMHexagonCodeGen LLVMHexagonAsmParser LLVMHexagonDesc LLVMHexagonInfo 
        # LLVMBPFDisassembler LLVMBPFCodeGen LLVMBPFAsmParser LLVMBPFDesc LLVMBPFInfo 
        # LLVMARMDisassembler LLVMARMCodeGen LLVMARMAsmParser LLVMARMDesc LLVMARMUtils LLVMARMInfo 
        # LLVMAMDGPUDisassembler LLVMAMDGPUCodeGen LLVMMIRParser LLVMAMDGPUAsmParser 
        # LLVMAMDGPUDesc LLVMAMDGPUUtils LLVMAMDGPUInfo 
        # LLVMAArch64Disassembler  LLVMAArch64CodeGen 
        LLVMMCDisassembler
        LLVMCFGuard LLVMGlobalISel LLVMSelectionDAG LLVMAsmPrinter LLVMDebugInfoDWARF 
        # LLVMAArch64AsmParser LLVMAArch64Desc LLVMAArch64Utils LLVMAArch64Info 
        LLVMMCJIT 
        LLVMipo LLVMFrontendOpenMP
        LLVMExecutionEngine LLVMRuntimeDyld LLVMLTO LLVMPasses LLVMObjCARCOpts 
        LLVMInstrumentation LLVMVectorize LLVMLinker LLVMIRReader LLVMAsmParser LLVMCodeGen LLVMTarget 
        LLVMScalarOpts LLVMInstCombine LLVMBitWriter LLVMAggressiveInstCombine LLVMTransformUtils 
        LLVMAnalysis LLVMProfileData LLVMObject LLVMTextAPI LLVMMCParser 
        LLVMMC LLVMDebugInfoCodeView LLVMDebugInfoMSF LLVMBitReader 
        LLVMCore LLVMRemarks LLVMBitstreamReader LLVMBinaryFormat LLVMSupport LLVMDemangle LLVMOption
    )

    # System libs that LLVM depend on.
    # See `llvm-config --system-libs`
    if (APPLE)
        set(SYSTEM_LIBS pthread)
    elseif (UNIX)
        set(SYSTEM_LIBS pthread dl)
    endif()

    if (${CMAKE_GENERATOR} STREQUAL "Unix Makefiles")
        set(BUILD_COMMAND $(MAKE))
    else()
        set(BUILD_COMMAND cmake --build <BINARY_DIR> --config Release)
    endif()

    set(cxx_flags "-std=c++17")
    message(STATUS "[+] used LLVM from ${CMAKE_BINARY_DIR}/deps")

    set(prefix ${CMAKE_BINARY_DIR}/deps)
    message(STATUS "[+] build mode: " ${CMAKE_BUILD_TYPE})
    
    set(LLVM_SRC_DIR ${prefix}/llvm-project)
    message(STATUS "[+] LLVM_SRC_DIR: " ${LLVM_SRC_DIR})

    include(ExternalProject)
    ExternalProject_Add(llvm
        PREFIX ${prefix}
        DOWNLOAD_NAME llvm-project-13.0.0.tar.xz
        DOWNLOAD_DIR ${prefix}/downloads
        URL https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.0/llvm-project-13.0.0.src.tar.xz
        URL_HASH SHA256=6075ad30f1ac0e15f07c1bf062c1e1268c241d674f11bd32cdf0e040c71f2bf3
        DOWNLOAD_NO_PROGRESS TRUE
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
                    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                    -DLLVM_ENABLE_TERMINFO=OFF  # Disable terminal color support
                    -DLLVM_ENABLE_ZLIB=OFF      # Disable compression support -- not needed at all
                    -DLLVM_TARGETS_TO_BUILD=NVPTX
                    -DLLVM_ENABLE_PROJECTS='clang'
                    -DLLVM_INCLUDE_EXAMPLES=OFF
                    -DLLVM_INCLUDE_TESTS=OFF
                    -DLLVM_INCLUDE_TOOLS=ON
                    # -DLLVM_ABI_BREAKING_CHECKS=FORCE_ON
                    -DCMAKE_CXX_FLAGS=${cxx_flags}
        LOG_CONFIGURE TRUE
        SOURCE_SUBDIR llvm # since we download `llvm-project` rather than `llvm`, we must build tools in `llvm/` sub-dir.
        BUILD_COMMAND   ${BUILD_COMMAND}
        # INSTALL_COMMAND cmake --build <BINARY_DIR> --config Release --target -- all  install
        INSTALL_COMMAND cmake --build <BINARY_DIR> --config Release --target install
        LOG_INSTALL TRUE
        EXCLUDE_FROM_ALL TRUE # build LLVM again with $make llvm
    )

    ExternalProject_Get_Property(llvm INSTALL_DIR)
    set(LLVM_LIBRARY_DIRS ${INSTALL_DIR}/lib)
    set(LLVM_INCLUDE_DIRS ${INSTALL_DIR}/include)
    file(MAKE_DIRECTORY ${LLVM_INCLUDE_DIRS})  # Must exist.

    foreach(LIB ${LIBS})
        list(APPEND LIBFILES "${LLVM_LIBRARY_DIRS}/${CMAKE_STATIC_LIBRARY_PREFIX}${LIB}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    endforeach()

    # Pick one of the libraries to be the main one. It does not matter which one
    # but the imported target requires the IMPORTED_LOCATION property.
    list(GET LIBFILES 0 MAIN_LIB)
    list(REMOVE_AT LIBFILES 0)
    set(LIBS ${LIBFILES} ${SYSTEM_LIBS})
    set(LLVM_DIR "${INSTALL_DIR}" CACHE INTERNAL "LLVM ROOT Directory" )
    # endif()

    message(STATUS "[+] Using LLVMConfig.cmake in: ${LLVM_DIR}")

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Clang needs this to build LLVM. Weird that the GCC does not.
        set(DEFINES __STDC_LIMIT_MACROS __STDC_CONSTANT_MACROS)
    endif()

    # Create the target representing
    add_library(LLVM::JIT STATIC IMPORTED)
    set_property(TARGET LLVM::JIT PROPERTY IMPORTED_CONFIGURATIONS Release)
    set_property(TARGET LLVM::JIT PROPERTY IMPORTED_LOCATION_RELEASE ${MAIN_LIB})
    set_property(TARGET LLVM::JIT PROPERTY INTERFACE_COMPILE_DEFINITIONS ${DEFINES})
    set_property(TARGET LLVM::JIT PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${LLVM_INCLUDE_DIRS})
    set_property(TARGET LLVM::JIT PROPERTY INTERFACE_LINK_LIBRARIES ${LIBS})

    if (TARGET llvm)
        add_dependencies(LLVM::JIT llvm)
    endif()

endfunction()