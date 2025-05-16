#pragma once

// #define DEBUG_FLAG
// #define MAZE_ENABLE

#define BLOCK_X 64//256 //128
#define BLOCK_Y 1
#define BLOCK_Z 1

#define GRID_X 16//32 // 512
#define GRID_Y 1
#define GRID_Z 1

#define NSTREAMS 1

#define NJOBS \
  (NSTREAMS * BLOCK_X * BLOCK_Y * BLOCK_Z * GRID_X * GRID_Y * GRID_Z)

// EVM memory&stack size in bytes
#define MAP_SIZE_POW2 12
#define MAP_SIZE (1 << MAP_SIZE_POW2)

#define EVM_MEM_SIZE 728

#define EVM_STK_WIDTH 32
#define EVM_STK_SIZE (EVM_STK_WIDTH * 256)

// #define EXEC_FAIL 0xfee1dead

#define EXEC_CRASH 0xdeaddead
#define EXEC_HNB 0xabcdabcd

// bitmap signature for the sanitizer
// #define INTSAN_SIG 0x01010101
// #define RENSAN_SIG 0x02020202

enum ExecStatus  {
  EXECNONE = 0, EXECREVERTED, EXECINTERESTING, EXECBUGGY
};

typedef enum SigByte_enum {
  SIG_EXEC_NONE,
  SIG_INTERESTING,
  SIG_EXEC_FAIL,
  SIG_REN_SAN, // RE
  SIG_INT_SAN, // IB
  SIG_ORG_SAN, // TO
  SIG_SUC_SAN, // SC
  SIG_BSD_SAN, // BD
  SIG_MSE_SAN, // ME
  SIG_ELK_SAN, // EL
  SIG_AWR_SAN, // AW
  SIG_CHJ_SAN, // CH
  SIG_MSS_SAN, // MS
} SigByte;


typedef enum BugClass_enum {
  AssertionFailure,
  ArbitraryWrite,
  BlockstateDependency,
  BlockstateDependencySFuzz,
  BlockstateDependencyILF,
  BlockstateDependencyMythril,
  BlockstateDependencyManticore,
  ControlHijack,
  EtherLeak,
  IntegerBug,
  IntegerBugSFuzz,
  IntegerBugMythril,
  IntegerBugManticore,
  MishandledException,
  MishandledExceptionSFuzz,
  MishandledExceptionILF,
  MishandledExceptionMythril,
  MishandledExceptionManticore,
  MultipleSend,
  Reentrancy,
  ReentrancySFuzz,
  ReentrancyILF,
  ReentrancyMythril,
  ReentrancyManticore,
  SuicidalContract,
  TransactionOriginUse,
  FreezingEther,
  RequirementViolation
} BugClass;


typedef enum SnapOption_enum {
  SNAPSHOT_L1,
  SNAPSHOT_L2,
} SnapOption;

#define SELFADDR_REF "SELFADDRESS"
// #define CALLER_REF "CALLER"
#define ORIGIN_REF "ORIGIN"
#define TIMESTAMP_REF "TIMESTAMP"
#define BLOCKNUM_REF "BLOCKNUM"
#define BALANCECALLER_REF "BALANCECALLER"
#define MSGVALUE_REF "MSGVALUE"

#define GASLIMIT_MAX (210000)
#define EVMCODE_SIZE (32 * 1024) /* 32 KB*/
#define EVMCODE_REG "__evmCode"
#define EVMCODESIZE_REG "__evmCodeSize"
#define EVMRUNTIMECODE "__evmRuntimeCode"

#define JMPTARGET_PTR_REG "JMP_TARGET_PTR"

#define BITMAPS_REG "__bitmaps"

// #define SIGNALS_REG "__signals"

#define DEBUG_BUFFER "__debug_buffer"
#define DEBUG_PCS "__debug_pcs"
#define DEBUG_PCS_MAX 8096
#define DEBUG_PCS_PTR "__debug_pc"

#define FN_CALLDATAMUTATOR "__mutateCalldata"

#define ONE_SNAP_LEN 128 /* the number of slots of each storage */

#define L1SNAP_VEC "l1snaps"
#define L1SNAP_LEN_VEC "l1snap_lens"

#define L2SNAP_VEC "l2snaps"
#define L2SNAP_LEN_VEC "l2snap_lens"

#define VIRGIN_TB_REG "__virgin_bits"
#define COV_TB_REG "__cov_bits"

#define SIGNAL_VEC_LEN (256+1)

#define MEM_REF_REG "MEMORY"
#define STK_REF_REG "STACK"
#define STK_DEP_PTR_REG "STACK_DEP_PTR"

#define CUDA_KERNEL_FUNC "main_contract"
#define CUDA_KERNEL_DEPLOYER_FUNC "main_deployer"


#define CUTHREAD_FUNC "get_thread_id"
#define CUDA_CONTRACT_FUNC "contract"
#define CUDA_DEPLOY_FUNC "deployer"

// Macro to aligned up to the memory size in question
#define MEMORY_ALIGNMENT 4096
#define ALIGN_UP(x, size) (((unsigned long)x + (size - 1)) & (~(size - 1)))
/* usage
UA = (float *)malloc(bytes + MEMORY_ALIGNMENT);
// We need to ensure memory is aligned to 4K (so we will need to padd memory
// accordingly)
a = (float *)ALIGN_UP(UA, MEMORY_ALIGNMENT);
*/

/*
 +-----------------+----------------+-----------------------------+
 | calldataSize 4B | calldata ....  |           ... |
 +-----------------+----------------+-----------------------------+
 <--------- transaction#1 ----------><----- transaction #n ------->
*/

#define CALLDATA_SIZE (2048)
#define TRANSACTIONS_MAX_LENGTH 1

/* (ALIGN_UP(CALLDATA_SIZE, 8))  in 64 bits*/
#define TRANSACTION_SIZE (CALLDATA_SIZE)
#define SEED_SIZE (TRANSACTION_SIZE * TRANSACTIONS_MAX_LENGTH)

/* for the fuzzer */
#define USE_COLOR

/* Comment out to disable fancy ANSI boxes and use poor man's 7-bit UI: */

#define FANCY_BOXES

#define EXEC_TIMEOUT 1000
// 8GB
#define MEM_LIMIT 257698037


#define HASH_CONST          0xa5b35705
