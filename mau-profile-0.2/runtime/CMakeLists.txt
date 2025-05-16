project(runtime LANGUAGES C)

set(CMAKE_C_COMPILER ${LLVM_DIR}/bin/clang)
message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")

set(LLVM_LINKER ${LLVM_DIR}/bin/llvm-link)

set(SOURCES_LIBS
  ptx_builtins.h
  random.h
  keccak256.c keccak256.h
  mutate.c mutate.h
  storage.c storage.h
  rt.o.c
)

set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-flto -S")
add_executable(runtime rt.o.c ${SOURCES_LIBS})
target_compile_features(runtime PRIVATE cxx_std_17)
target_compile_options(runtime PUBLIC ${CMAKE_C_FLAGS}
                       -m64 --target=nvptx64-nvidia-cuda -O3 -g0 -funroll-loops
                       -Wall -Wno-pointer-sign -fPIC -emit-llvm)

set(BITCODE_DIR "${PROJECT_BINARY_DIR}/CMakeFiles/runtime.dir/")
message(STATUS "[+] bitcode directory: ${BITCODE_DIR}")

add_custom_command(TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${LLVM_LINKER} --only-needed ${BITCODE_DIR}/rt.o.c.o ${BITCODE_DIR}/keccak256.c.o ${BITCODE_DIR}/mutate.c.o -o ${CMAKE_BINARY_DIR}/rt.o.bc
  VERBATIM
)

# export a bash to control ptxsema and other tools
add_custom_command(TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND echo "${CMAKE_BINARY_DIR}/ptxsema $1 -o .code.ll --hex --dump && ${LLVM_DIR}/bin/llvm-link ${CMAKE_BINARY_DIR}/rt.o.bc .code.ll -o .ptx.bc && ${CMAKE_SOURCE_DIR}/scripts/llc-16 -mcpu=sm_86 .ptx.bc -o $2" >> ${CMAKE_BINARY_DIR}/mau && chmod +x ${CMAKE_BINARY_DIR}/mau
  VERBATIM
)
