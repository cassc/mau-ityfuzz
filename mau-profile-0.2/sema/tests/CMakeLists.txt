project(tests LANGUAGES CXX)

# set (CUDA_PATH "/usr/local/cuda")

set(PROJECT_TEST_BINARY_DIR ${CMAKE_SOURCE_DIR}/sema/tests/)
set(CMAKE_CXX_STANDARD 11)

find_program(BASH_EXECUTABLE NAMES bash REQUIRED)
set(LIFTER_BINARY_PATH ${CMAKE_BINARY_DIR}/sema/src/standalone-ptxsema)

# message(STATUS "TestBinary = ${PROJECT_TEST_BINARY_DIR}")
# message(STATUS "LIFTER_BINARY_PATH  = ${LIFTER_BINARY_PATH}")
include_directories("/usr/local/cuda-11.4/include/")

add_executable(loader loader.cpp)
target_link_libraries(loader cuda)

# ploader
add_executable(ploader ploader.cpp)
target_link_libraries(ploader cuda)

set(LOADER_BINARY_PATH ${CMAKE_BINARY_DIR}/sema/tests/loader)

if(MSVC)
  set(SYSTEM_LIBS "")
elseif(APPLE)
  set(SYSTEM_LIBS stdc++ curses z pthread m)
else()
  set(SYSTEM_LIBS stdc++ ncurses z pthread dl m)
endif()

set(RENSAN_SIG          "03")
set(INTSAN_SIG          "04")
set(ORGSAN_SIG          "05")

# ================================= arithmetic.hex =========================================
# test 256bits arithmetic operations
add_test(
  NAME compile.arithmetic
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/ptx.sh
  ${LIFTER_BINARY_PATH}
  ${PROJECT_TEST_BINARY_DIR}/contracts/arithmetic.hex
)

# uadd(2, 3) == 5
add_test(
  NAME arithmetic.add.u256
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  "0x0000000000000000000000000000000000000000000000000000000000000005"
  0
  "0x00"
  0
  "0x8d41c17d00000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000003"
)


# smul(2, -2) == 4
add_test(
  NAME arithmetic.mul.i256
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc"
  0
  "0x00"
  0
  "0x2108a63e0000000000000000000000000000000000000000000000000000000000000002fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe"
)

# sdiv(9, -1) = -9
add_test(
  NAME arithmetic.sdiv.i256
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7"
  0
  "0x00"
  0
  "0x074898e50000000000000000000000000000000000000000000000000000000000000009ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
)



# =================================== string.hex ===========================================
add_test(
  NAME "compile.string"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/ptx.sh
  ${LIFTER_BINARY_PATH}
  "${PROJECT_TEST_BINARY_DIR}/contracts/string.hex"
)

add_test(
  NAME "string.setProvenanceHash"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  "000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000046162636400000000000000000000000000000000000000000000000000000000"
  0
  "0x00"
  0
  "0x10969523000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000046162636400000000000000000000000000000000000000000000000000000000"
)

# # =================================== storage.hex ===========================================
add_test(
  NAME "compile.storage"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/ptx.sh
  ${LIFTER_BINARY_PATH}
  "${PROJECT_TEST_BINARY_DIR}/contracts/storage.hex"
)

add_test(
  NAME "storage.set"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  "000000000000000000000000000000000000000000000000000000000000000e"
  0
  "0x00000000000000000000000000000000000000000000000000000000000000020000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4"
  0
  "0x60fe47b1000000000000000000000000000000000000000000000000000000000000000c" 
)

add_test(
  NAME "storage.set.get.set.get"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  "000000000000000000000000000000000000000000000000000000000000000e"
  0
  "0x00000000000000000000000000000000000000000000000000000000000000020000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4"
  0
  "0x60fe47b1000000000000000000000000000000000000000000000000000000000000000c" 
  0
  "0x0dbe671f"
)

# =================================== erc20.hex ===========================================
add_test(
  NAME "compile.erc20"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/ptx.sh
  ${LIFTER_BINARY_PATH}
  "${PROJECT_TEST_BINARY_DIR}/contracts/erc20.hex"
)

add_test(
  NAME "erc20.decimals"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  "0000000000000000000000000000000000000000000000000000000000000012"
  0
  "0x00"
  0
  "0x313ce567" 
)


# =================================== overflow_mapping.hex ===========================================
add_test(
  NAME "compile.overflow_mapping"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/ptx.sh
  ${LIFTER_BINARY_PATH}
  "${PROJECT_TEST_BINARY_DIR}/contracts/integer_overflow_mapping_sym_1.bin"
)

add_test(
  NAME "overflow_mapping.init"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" 
  0
  "0x00"
  0
  "0xa5843f0800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001" 
)

# =================== san.intsan.sub. =================
add_test(
  NAME "compile.san.overflow_mapping"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/ptx.sh
  ${LIFTER_BINARY_PATH}
  "${PROJECT_TEST_BINARY_DIR}/contracts/integer_overflow_mapping_sym_1.bin"
  "--fsanitize=intsan"
)

add_test(
  NAME "san.overflow_mapping.init.begine"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  "0000000000000000000000000000000000000000000000000000000000000000" 
  0
  "0x00"
  0
  "0xa5843f0800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000" 
)

add_test(
  NAME "san.overflow_mapping.init.exploit"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  ${INTSAN_SIG}
  0
  "0x00"
  0
  "0xa5843f0800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001" 
)


# =================== int.san ==============================
add_test(
  NAME compile.san.arithmetic
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/ptx.sh
  ${LIFTER_BINARY_PATH}
  ${PROJECT_TEST_BINARY_DIR}/contracts/arithmetic.hex
  "--fsanitize=intsan"
)

add_test(
  NAME arithmetic.san.add
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  ${INTSAN_SIG}
  0
  "0x00"
  0
  "0x8d41c17d0000000000000000000000000000000000000000000000000000000000000002ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
)

add_test(
  NAME arithmetic.san.umul.i256
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  ${INTSAN_SIG}
  0
  "0x00"
  0
  "0x904d5ed80000000000000000000000000000000000000000000000000000000000000002fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe"
)

add_test(
  NAME arithmetic.san.usub.i256
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  ${INTSAN_SIG}
  0
  "0x00"
  0
  "0xd8b87f2e00000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000002"
)

# # =================================== reentrance.hex ===========================================
add_test(
  NAME "compile.reentrance"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/ptx.sh
  ${LIFTER_BINARY_PATH}
  "${PROJECT_TEST_BINARY_DIR}/contracts/reentrance.hex"
  "-txs-len=3 --disable-instrument"
)

add_test(
  NAME "reentrance.exploit"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  "0000000000000000000000000000000000000000000000000000000000000009" 
  10
  "0xe2c41dbc"
  1
  "0x155dd5ee0000000000000000000000000000000000000000000000000000000000000001"
  0
  "0x27e235e30000000000000000000000009da6739b8e34a561a007aa230059922ed2dcd5bb"   
)

add_test(
  NAME "compile.vul.reentrance"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/ptx.sh
  ${LIFTER_BINARY_PATH}
  "${PROJECT_TEST_BINARY_DIR}/contracts/reentrance.hex"
  "-txs-len=3 --fsanitize=rensan --disable-instrument"
)

add_test(
  NAME "vul.reentrance.exploit"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  ${RENSAN_SIG}
  10
  "0xe2c41dbc"
  0
  "0x155dd5ee0000000000000000000000000000000000000000000000000000000000000001"
  0
  "0x27e235e3"   
)


add_test(
  NAME "compile.xcall"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/ptx.sh
  ${LIFTER_BINARY_PATH}
  "${PROJECT_TEST_BINARY_DIR}/contracts/xcall.hex"
  "-txs-len=3 --fsanitize=rensan --disable-instrument"
)

add_test(
  NAME "xcall.withdraw"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  ${RENSAN_SIG}
  1
  "0xe2c41dbc"
  0
  "0x24600fc3"
  0
  "0x0dbe671f"  
)



# =================================== hashmapping.hex ===========================================
add_test(
  NAME "compile.hashmapping"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/ptx.sh
  ${LIFTER_BINARY_PATH}
  "${PROJECT_TEST_BINARY_DIR}/contracts/hashmapping.hex"
  "-txs-len=2"
)

add_test(
  NAME "hashmapping.deposit.balances"
  COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/eval.sh
  ${LOADER_BINARY_PATH}
  "000000000000000000000000000000000000000000000000000000000000000a" 
  10
  "0xe2c41dbc"
  0
  "0x27e235e30000000000000000000000009da6739b8e34a561a007aa230059922ed2dcd5bb"
)

# ================================= ploader ==================================

# add_test(
#   NAME "pthread.example"
#   COMMAND ${BASH_EXECUTABLE} ${PROJECT_TEST_BINARY_DIR}/ptx.sh
#   ${LIFTER_BINARY_PATH}
#   "${PROJECT_TEST_BINARY_DIR}/contracts/storage.hex"
# )


# add_test(
#   NAME pthread.add
#   COMMAND ${ENV_EXECUTABLE} ${CMAKE_BINARY_DIR}/sema/tests/ploader
#   ./kernel.ptx
#   0
#   "0x60fe47b1000000000000000000000000000000000000000000000000000000000000000c" 
# )