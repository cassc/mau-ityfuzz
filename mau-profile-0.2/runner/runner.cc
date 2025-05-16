
#include <string.h>
#include <set>
#include <cstdio>
#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <map>

#include <cuda.h>
#include <cuda_runtime_api.h>


#include "debug.h"
#include "fuzzer_config.h"
#include "type.h"


static CUcontext cuCtx;
static CUmodule cuModule;

static std::set<uint64_t> BugSet;
// static uint64_t TotalCrashes = 0;

// static uint64_t *hSignals = nullptr;  // host pinned
// static bool DuGains[NJOBS];
static uint64_t Coverage = 0;
static uint32_t BytecodeSize = 0;

static CUfunction fnMain, fnDeploy, fnEval, fnMutate = NULL;
static CUdeviceptr dEvmCode, dEvmCodeSize, dnothing, dSnapMap, dSeeds = 0;
// static CUdeviceptr dSignals = 0;
static CUdeviceptr dSig, dHnb = 0;
uint8_t *hnbs;

static std::map<uint32_t, uint32_t> storage_pos_map;


std::string BugToTag(SigByte Sig) {
  switch (Sig) {
  case SIG_REN_SAN:
    return "Reentrancy";
  case SIG_INT_SAN:
    return "IntegerBug";
  case SIG_ORG_SAN:
    return "TransactionOriginUse";
  case SIG_SUC_SAN:
    return "SuicidalContract";
  case SIG_BSD_SAN:
    return "BlockstateDependency";
  case SIG_MSE_SAN:
    return "MishandledException";
  case SIG_ELK_SAN:
    return "EtherLeak";
  case SIG_AWR_SAN:
    return  "ArbitraryWrite";
  case SIG_CHJ_SAN:
    return "ControlHijack";
  case SIG_MSS_SAN:
    return "MultipleSend";
  default:
    FATAL("Invalid Bug Tag %d", Sig);
  }
  return "";
}

static const char *_cudaGetErrorEnum(CUresult error) {
  static char unknown[] = "<unknown>";
  const char *ret = NULL;
  cuGetErrorName(error, &ret);
  return ret ? ret : unknown;
}

#define cuCheck(result)                                                        \
  do {                                                                         \
    if (result != CUDA_SUCCESS) {                                              \
       PFATAL("[Cuda] %d(%s)\n",                                               \
            static_cast<unsigned int>(result), _cudaGetErrorEnum(result));     \
    }                                                                          \
  } while (0)


extern "C"  uint32_t __declspec(dllexport) cuVersion() {
  return 1000;
}

extern "C"  void __declspec(dllexport) HelloWorld() {
	int a = 12;
	printf("Hello world, invoked by F#! a = %d\n", a);
}

void showDevice(CUdevice dev) {
  int data; 
  printf("---------------- device info ------------------\n");
  cuDeviceGetAttribute(&data, CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK,
                       dev);
  OKF("Shared Mem:  %d KB / block", data >> 10);

  cuDeviceGetAttribute(
      &data, CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_MULTIPROCESSOR, dev);
  OKF("Shared Mem:  %d KB / grid", data >> 10);

  cuDeviceGetAttribute(&data, CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY, dev);
  OKF("Total Const Mem:  %d MB", data >> 20);

  cuDeviceGetAttribute(&data, CU_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH, dev);
  OKF("GPU Mem Bus Width:  %d bits", data);

  size_t memfree, memtotal;
  cuMemGetInfo(&memfree, &memtotal);
  OKF("Total mem = %zu GB  Free mem = %zu GB", memtotal >> 30, memfree >> 30);
}

void _dumpStorage(CUdeviceptr d_len, CUdeviceptr d_snap) {
  uint8_t slotCnt;
  cuCheck(cuMemcpyDtoH(&slotCnt, d_len, sizeof(uint8_t)));
  if (slotCnt > ONE_SNAP_LEN)
    WARNF("Increase the SnapshotSize. Used slots %d/%d.", slotCnt, ONE_SNAP_LEN);
  
  slot_t *storage = new slot_t[slotCnt];
  cuCheck(cuMemcpyDtoH(storage, d_snap, sizeof(slot_t) * slotCnt));

  printf("------ %u slots ------\n", slotCnt); 
  for (auto i = 0; i < slotCnt; i++) {
    printf("Key:");
    printbytes((uint8_t *)&storage[i].key[0], 32);
    printf("Val:");
    printbytes((uint8_t *)&storage[i].val[0], 32);
  }
  delete [] storage; 
}

extern "C" void __declspec(dllexport) cuDumpStorage(size_t threadId) {
  // dump L2snap[threadId] => initial volume storage
  CUdeviceptr dL1SnapLenVec, dL1SnapVec;
  cuCheck(cuModuleGetGlobal(&dL1SnapLenVec, nullptr, cuModule, L1SNAP_LEN_VEC));
  cuCheck(cuModuleGetGlobal(&dL1SnapVec, nullptr, cuModule, L1SNAP_VEC));
  _dumpStorage(dL1SnapLenVec + threadId, dL1SnapVec + sizeof(slot_t) * ONE_SNAP_LEN * threadId);

  // dump L1snap[threadId] => runtime storage for #threadId
  // CUdeviceptr dL1SnapLenVec, dL1SnapVec;
  // cuCheck(cuModuleGetGlobal(&dL1SnapLenVec, nullptr, cuModule, L1SNAP_LEN_VEC));
  // cuCheck(cuModuleGetGlobal(&dL1SnapVec, nullptr, cuModule, L1SNAP_VEC));
  // _dumpStorage(dL1SnapLenVec + threadId, dL1SnapVec + sizeof(slot_t) * ONE_SNAP_LEN * threadId);

  // uint8_t slotCnt;
  // slot_t *storage = new slot_t[slotCnt];

  // // dump L1snap[threadId] => runtime storage for #threadId
  // CUdeviceptr dL1SnapLenVec, dL1SnapVec;
  // cuCheck(cuModuleGetGlobal(&dL1SnapLenVec, nullptr, cuModule, L1SNAP_LEN_VEC));
  // cuCheck(cuModuleGetGlobal(&dL1SnapVec, nullptr, cuModule, L1SNAP_VEC));
  // cuCheck(cuMemcpyDtoH(&slotCnt, dL1SnapLenVec + threadId, sizeof(uint8_t)));
  // if (slotCnt > ONE_SNAP_LEN) WARNF("Increase the SnapshotSize. Used slots %d/%d.", slotCnt, ONE_SNAP_LEN);
  // cuCheck(cuMemcpyDtoH(storage, dL1SnapVec + sizeof(slot_t) * ONE_SNAP_LEN * threadId, sizeof(slot_t) * slotCnt));
  // printf("------ %u slots ------\n", slotCnt); 
  // for (auto i = 0; i < slotCnt; i++) {
  //   printf("Key:");
  //   printbytes((uint8_t *)&storage[i].key[0], 32);
  //   printf("Val:");
  //   printbytes((uint8_t *)&storage[i].val[0], 32);
  // }
  // delete [] storage; 
}

extern "C" void __declspec(dllexport) InitCudaCtx(const int Dev, 
                                                  const char *pathToKernel) {
#ifdef DEBUG_FLAG
    OKF("Initiating %s on GPU device# %d\n", pathToKernel, Dev);
#endif

  CUdevice device;
  int devCount;
  cuCheck(cuInit(0));
  cuCheck(cuDeviceGetCount(&devCount));
  if (Dev >= devCount) 
    printf("There are # %d GPUs, but you are using an invalid one.\n"
        "CUDA_DEVICE_ID: %d\n", devCount, Dev);
  cuCheck(cuDeviceGet(&device, Dev));
  cuCheck(cuCtxCreate(&cuCtx, CU_CTX_SCHED_BLOCKING_SYNC, device));
  cuCheck(cuCtxSetLimit(CU_LIMIT_MALLOC_HEAP_SIZE, 257698037));
  cuCheck(cuCtxSetLimit(CU_LIMIT_STACK_SIZE, 4096*2));
  // showDevice(device)
  printf("Loading Kernel %s\n", pathToKernel);
  cuCheck(cuModuleLoad(&cuModule, pathToKernel));
  printf("Loaded Kernel\n");
  CUdeviceptr dstates;
  cuCheck(cuModuleGetGlobal(&dstates, nullptr, cuModule, "cuda_states"));
  srand((int)time(NULL));
  int32_t rands[NJOBS];
  for (uint32_t i = 0; i < NJOBS; i++) {
    rands[i] = rand();
  }
  cuCheck(cuMemcpyHtoD(dstates, &rands[0], NJOBS * sizeof(int32_t)));  // inititated to the full mask
  cuCheck(cuModuleGetFunction(&fnMain, cuModule, CUDA_KERNEL_FUNC));
  return;
}

extern "C" void __declspec(dllexport) DestroyCuda() {
  cuModuleUnload(cuModule);
  cuCtxDestroy(cuCtx);
  cudaDeviceReset();
}

extern "C" bool __declspec(dllexport) setEVMEnv(
    uint8_t *To, uint8_t *Timestamp, uint8_t*Blocknum) {
  // printf("\tcaller: ");
  // printbytes(From, 20);
  // printf("\tto: ");
  // printbytes(To, 20);
  // printf("\ttimestamp: ");
  // printbytes(Timestamp, 32);
  // printf("\tnumber: ");
  // printbytes(Blocknum, 32);

  CUdeviceptr dOrigin, dTo, dTimestamp, dBlockNum;
  // if (cuModuleGetGlobal(&dCaller, nullptr, cuModule, CALLER_REF) == CUDA_SUCCESS) {
  //   cuCheck(cuMemcpyHtoD(dCaller, From, 20));
  // }
  // if (cuModuleGetGlobal(&dOrigin, nullptr, cuModule, ORIGIN_REF) == CUDA_SUCCESS) {
  //   cuCheck(cuMemcpyHtoD(dOrigin, From, 20));
  // }

  if (cuModuleGetGlobal(&dTo, nullptr, cuModule, SELFADDR_REF) == CUDA_SUCCESS) {
    cuCheck(cuMemcpyHtoD(dTo, To, 20));
  }

  if (cuModuleGetGlobal(&dTimestamp, nullptr, cuModule, TIMESTAMP_REF) == CUDA_SUCCESS) {
    cuCheck(cuMemcpyHtoD(dTimestamp, Timestamp, 32));
  } 

  if (cuModuleGetGlobal(&dBlockNum, nullptr, cuModule, BLOCKNUM_REF) == CUDA_SUCCESS) {
    cuCheck(cuMemcpyHtoD(dBlockNum, Blocknum, 32));
  } 
  return true;
}

extern "C" void __declspec(dllexport) cuMallocAll() {
  // if (!hSignals)  hSignals = (uint64_t *)malloc(SIGNAL_VEC_LEN * sizeof(uint64_t));
  
  cuCheck(cuMemAlloc(&dSeeds, NJOBS * SEED_SIZE)); // for all threads

  // cuCheck(cuModuleGetGlobal(&dSignals, nullptr, cuModule, SIGNALS_REG));

  CUdeviceptr dBitmaps;
  cuCheck(cuModuleGetGlobal(&dBitmaps, nullptr, cuModule, BITMAPS_REG));
  CUdeviceptr dOne, dOneBitmap;
  for (auto i = 0; i < NJOBS; ++i) {
    cuCheck(cuMemAlloc(&dOne, MAP_SIZE));
    cuCheck(cuMemsetD8(dOne, 0, MAP_SIZE));
    dOneBitmap = dBitmaps + i * sizeof(uint8_t *);
    cuCheck(cuMemcpyHtoD(dOneBitmap, &dOne, 8));
  }

  CUdeviceptr dVirgin, dCovBits;
  cuCheck(cuModuleGetGlobal(&dVirgin, nullptr, cuModule, VIRGIN_TB_REG));
  cuMemsetD8(dVirgin, 0xff, MAP_SIZE);  // inititated to the full mask
  
  // cuCheck(cuModuleGetGlobal(&dCovBits, nullptr, cuModule, COV_TB_REG));
  // cuMemsetD8(dCovBits, 0, MAP_SIZE);
  
  cuCheck(cuMemAlloc(&dnothing, 1)); 
  cuCheck(cuMemAlloc(&dSig, sizeof(uint64_t)));
  
  // pinned
  cuCheck(cuMemHostAlloc((void**)&hnbs, NJOBS * sizeof(uint8_t), CU_MEMHOSTALLOC_DEVICEMAP));
  // cuCheck(cuMemHostGetDevicePointer(&dHnb, hnbs, 0));
  cuCheck(cuModuleGetGlobal(&dHnb, nullptr, cuModule, "__hnbs"));
  cuCheck(cuModuleGetGlobal(&dSnapMap, nullptr, cuModule, "__snap_map"));
}

extern "C" void __declspec(dllexport) cuFreeAll() {
  // CUdeviceptr dBitmaps;
  // cuCheck(cuModuleGetGlobal(&dBitmaps, nullptr, cudaModule, BITMAPS_REG));
  // CUdeviceptr dOne; // fixme
  // // for (auto i = 0; i < NJOBS; ++i) {
  // //   cuCheck(cuMemcpyDtoH(&dOne, dBitmaps + i * sizeof(uint8_t*), 8));
  // //   cuCheck(cuMemFree(dOne));
  // // }
  // cuCheck(cuMemFree(dSeeds));
  // cuCheck(cuMemFree(dnothing));
  
  // cuCheck(cuMemFree(dCov));
  // cuCheck(cuMemFree(dHnb));
}


extern "C" void __declspec(dllexport) cuAddCallerPool(uint8_t *added_caller, uint32_t pool_len) {
  CUdeviceptr d;
  // __assert(pool_len > 0, "pool_len must be larger than one.");
  printf("[-] cuAddCallerPool: copy from cpu to gpu ");
  printbytes(added_caller, 20);
  cuCheck(cuModuleGetGlobal(&d, nullptr, cuModule, "callers_pool"));
  cuCheck(cuMemcpyHtoD(d + 32 * pool_len - 32, added_caller, 20));

  cuCheck(cuModuleGetGlobal(&d, nullptr, cuModule, "callers_pool_len"));
  cuCheck(cuMemcpyHtoD(d, &pool_len, sizeof(pool_len)));
}

extern "C" void __declspec(dllexport) cuAddAddressPool(uint8_t *added_addr, uint32_t pool_len) {
  CUdeviceptr d;
  // __assert(pool_len > 0, "pool_len must be larger than one.");
  printf("[-] cuAddAddressPool: copy from cpu to gpu ");
  printbytes(added_addr, 20);
  cuCheck(cuModuleGetGlobal(&d, nullptr, cuModule, "addresses_pool"));
  cuCheck(cuMemcpyHtoD(d + 32 * pool_len - 32, added_addr, 20));

  cuCheck(cuModuleGetGlobal(&d, nullptr, cuModule, "addresses_pool_len"));
  cuCheck(cuMemcpyHtoD(d, &pool_len, sizeof(pool_len)));
}

extern "C" void __declspec(dllexport) cuLoadConstBytes(uint32_t idx, uint8_t *constant, uint8_t size) {
  CUdeviceptr d;

  uint32_t length = idx + 1;
  cuCheck(cuModuleGetGlobal(&d, nullptr, cuModule, "cbconstants_length"));
  cuCheck(cuMemcpyHtoD(d, &length, sizeof(length)));

  cuCheck(cuModuleGetGlobal(&d, nullptr, cuModule, "cbconstant_sizes"));
  cuCheck(cuMemcpyHtoD(d + 1 * idx, &size, 1));

  cuCheck(cuModuleGetGlobal(&d, nullptr, cuModule, "cbconstants"));
  cuCheck(cuMemcpyHtoD(d + 32 * idx, constant, size));

  printf("[CUDA] loading constant to [%d]: ", idx);
  printbytes(constant, size);
}

extern "C" void __declspec(dllexport) cuPreMutate(uint8_t *argTypes, const uint32_t argTypesLen) {
  CUdeviceptr dArgMap;
  cuCheck(cuModuleGetGlobal(&dArgMap, nullptr, cuModule, "argTypeMap"));
  cuCheck(cuMemcpyHtoD(dArgMap, argTypes, argTypesLen * sizeof(uint8_t))); 

  for (uint32_t i = 0; i < NJOBS; i++) 
    cuCheck(cuMemcpyDtoD(dSeeds + SEED_SIZE * i, dSeeds, SEED_SIZE));
}

extern "C" void __declspec(dllexport) cuExtSeeds(uint8_t *rawSeed, uint32_t size) {
  // load the seed and extend it
  if (size == 0) return;
  
  cuCheck(cuMemsetD8(dSeeds, 0, SEED_SIZE * sizeof(uint8_t)));
  if (rawSeed != NULL && size > 0)  {
#ifdef DEBUG_FLAG
    printf("cuExtSeeds: #%d Bytes ", size); 
    printbytes(rawSeed, size);
#endif
    cuCheck(cuMemcpyHtoD(dSeeds, rawSeed, size));
  }

  for (uint32_t i = 0; i < NJOBS; i++) 
    cuCheck(cuMemcpyDtoD(dSeeds + SEED_SIZE * i, dSeeds, SEED_SIZE));
}

std::map<uint32_t, uint32_t> storage_map;
extern "C" void __declspec(dllexport) cuSetStorageMap(uint32_t s_idx, uint32_t s_pos) {
  storage_map[s_idx] = s_pos;
}

extern "C" uint32_t __declspec(dllexport) cuGetStoragePos(uint32_t s_idx) {
  return storage_map[s_idx];
}

/// load the seed and extend it
extern "C" void __declspec(dllexport) cuLoadSeed(
    uint8_t const *caller_ptr, 
    uint8_t const *value_ptr, 
    uint8_t const *data_ptr, 
    uint32_t data_size, 
    uint32_t state_id,
    uint32_t thread) {
    
    uint32_t cuda_size = 68 + data_size;
    if (cuda_size > SEED_SIZE) {
      WARNF("[CUDA] the calldata (%d B) should be less than %d bytes\n", cuda_size, SEED_SIZE);
      return;
    }

    uint8_t seed[SEED_SIZE];
    memset(seed, 0, 32);
    memcpy(seed, caller_ptr, 20); // caller
    memcpy(seed + 32, value_ptr, 32); // callvalue
    *(uint32_t*)(seed + 64) = data_size; // calldatasize
    memcpy(seed + 68, data_ptr, data_size); // calldata
    cuCheck(cuMemcpyHtoD(dSeeds + SEED_SIZE * thread, seed, cuda_size));
    
    uint32_t state_pos = state_id; //storage_pos_map[state_id];
    cuCheck(cuMemcpyHtoD(dSnapMap + sizeof(state_pos) * thread, &state_pos, sizeof(state_pos)));
    
//   if (data_ptr != NULL && data_size > 0)  {
// #ifdef DEBUG_FLAG
    // printf("cuLoadSeed: #%d Bytes ", cuda_size); 
    // printbytes(seed, cuda_size);
// #endif
//     cuCheck(cuMemcpyHtoD(dSeeds + SEED_SIZE * pos, rawSeed, size));
//   }
}

extern "C" void __declspec(dllexport) cuMutate(uint32_t calldatasize) {

  if (calldatasize <= 4) 
    return;

  cuCheck(cuModuleGetFunction(&fnMutate, cuModule, "parallel_mutate"));
  void *KernelParams[] = {(void *)&dSeeds, (void *)&calldatasize}; 
  cuCheck(cuLaunchKernel(fnMutate, GRID_X, GRID_Y, GRID_Z, BLOCK_X, BLOCK_Y, BLOCK_Z, 0,
                          NULL, &KernelParams[0], NULL)); 
  cuCheck(cuCtxSynchronize());
  
  // CUdeviceptr d = 0;
  // uint32_t length = 0;
  // cuCheck(cuModuleGetGlobal(&d, nullptr, cuModule, "cbconstants_length"));
  // cuCheck(cuMemcpyDtoH(&length, d, sizeof(length)));
  // printf("[GPU] constants#%d ", length);
  // uint8_t cbsizes[2048];
  // cuCheck(cuModuleGetGlobal(&d, nullptr, cuModule, "cbconstant_sizes"));
  // cuCheck(cuMemcpyDtoH(cbsizes, d, 2048));

  // uint8_t cbdata[2048][32];
  // cuCheck(cuModuleGetGlobal(&d, nullptr, cuModule, "cbconstants"));
  // cuCheck(cuMemcpyDtoH(cbdata, d, 2048*32));

  // for (uint32_t i = 0; i < length; i++) {
  //   printbytes(cbdata[i], cbsizes[i]);
  // }
  // exit(0);
}

extern "C" bool __declspec(dllexport) cuDeployTx(
  uint64_t callvalue, 
  uint8_t *Buff, 
  uint32_t ArgSize) {
  // run the deployer
  cuCheck(cuModuleGetFunction(&fnDeploy, cuModule, CUDA_KERNEL_DEPLOYER_FUNC));
  
  // OKF("Obtaining BytecodeSize");
  if (!BytecodeSize) {
    // initiate the bytecode size
    cuCheck(cuModuleGetGlobal(&dEvmCode, nullptr, cuModule, EVMCODE_REG));
    cuCheck(cuModuleGetGlobal(&dEvmCodeSize, nullptr, cuModule, EVMCODESIZE_REG));
    uint32_t _BytecodeSize;
    cuCheck(cuMemcpyDtoH(&_BytecodeSize, dEvmCodeSize, sizeof(_BytecodeSize)));
    BytecodeSize = _BytecodeSize;
  }
  // OKF("Setting Bytecode Arguments");
  if (ArgSize && Buff != NULL) {
    uint32_t CodeSize = BytecodeSize + ArgSize;    
    if (CodeSize >= EVMCODE_SIZE) {
      WARNF("Bytecode+Args (%d + %d = %d B) is longer the upper (%d B)", BytecodeSize, ArgSize, CodeSize, EVMCODE_SIZE);
      CodeSize = EVMCODE_SIZE;
      ArgSize = CodeSize - BytecodeSize;
    }
    cuCheck(cuModuleGetGlobal(&dEvmCode, nullptr, cuModule, EVMCODE_REG));
    cuCheck(cuModuleGetGlobal(&dEvmCodeSize, nullptr, cuModule, EVMCODESIZE_REG));
    // printf("Deployer: BytecodeSize= %uB; CodeSize = %uB->\n", BytecodeSize, CodeSize);
    cuCheck(cuMemcpyHtoD(dEvmCodeSize, (uint8_t*)&CodeSize, sizeof(CodeSize))); // bytecode = evmcode + args
    cuCheck(cuMemcpyHtoD(dEvmCode + BytecodeSize, Buff, ArgSize)); // calldata
    printf("Constructor data: "); printbytes(Buff, ArgSize); printf("\n");
  }
  // cuCheck(cuMemcpyHtoD(dSeeds, &callvalue, 8)); 
  
  void *DeployerParams[] = {(void *)&dSeeds, (void *)&dnothing}; 
  cuCheck(cuLaunchKernel(fnDeploy, 1, 1, 1, 1, 1, 1, 0,
                         NULL, &DeployerParams[0], NULL)); 
  cuCheck(cuCtxSynchronize());  
  return true;
}

/// establish the parallel-data dseed
extern "C" bool __declspec(dllexport) 
cuDataCpy(const CUdeviceptr dev, uint64_t callvalue, uint8_t *buff, uint32_t size) {  
  // OKF("runner: copying");
  // printbytes(buff, size);
  uint8_t *tmp = new uint8_t[TRANSACTION_SIZE];
  memset(tmp, 0, TRANSACTION_SIZE);
  *(uint64_t*)tmp = callvalue;
  if (size + 12 > TRANSACTION_SIZE) {
    WARNF("Seed Size %d is longer than exptected %d", size, TRANSACTION_SIZE);
    size = TRANSACTION_SIZE - 12;
  }
  *(uint32_t*)(tmp + 8) = size;
  // OKF("runner: size =%d", size);
  if (size) memcpy(tmp + 12, buff, size);
  // OKF("runner: h2d");
  cuCheck(cuMemcpyHtoD(dev, tmp, TRANSACTION_SIZE)); 
  // OKF("runner: h2d completed");
  delete [] tmp;
  return true;
}

/// run the runtimer
// extern "C" bool __declspec(dllexport) 
// cuRunTxsInGroup(CUmodule cudaModule, uint64_t callvalue, 
//                 uint8_t *Buff, uint32_t ArgSize) {  
//   uint8_t tmp[512];
//   memset(tmp, 0, 512);
//   *(uint64_t*)tmp = callvalue;
//   *(uint32_t*)(tmp+8) = ArgSize;
//   memcpy(tmp + 12, Buff, ArgSize);
//   cuCheck(cuMemcpyHtoD(dSeeds, tmp, 512)); 

//   // printf("_--->");
//   // printbytes(tmp, 12 + ArgSize);
//   // printf("Test ptr = %llx; *ptr = %llx", dSeedsPtr, *dSeedsPtr);

//   uint32_t tidOffset = 0;
//   if (!fnMain)  cuCheck(cuModuleGetFunction(&fnMain, cudaModule, CUDA_KERNEL_FUNC));

//   void *KernelParams[] = {(void *)&dSeeds, (void *)&dnothing,             
//                           (void *)&dnothing, (void *)&tidOffset};
//   cuCheck(cuLaunchKernel(fnMain, 1, 1, 1, 1, 1, 1, 0,
//                           NULL, &KernelParams[0], NULL)); 
//   cuCheck(cuCtxSynchronize());
//   // cuDumpStorage(cudaModule, 0);
//   return true;
// }

/// run the runtimer
extern "C" uint64_t __declspec(dllexport) cuRunTxs(uint8_t *ArgTypeMap, uint32_t MapSize) { 
  
  CUdeviceptr dArgMap;
  cuCheck(cuModuleGetGlobal(&dArgMap, nullptr, cuModule, "argTypeMap"));
  cuCheck(cuMemcpyHtoD(dArgMap, &MapSize, sizeof(uint8_t))); // (uint8_t)MapSize
  if (MapSize)  cuCheck(cuMemcpyHtoD(dArgMap + 1, ArgTypeMap, MapSize*sizeof(uint8_t))); // argmap
  // printf(" ====> "); printbytes(ArgTypeMap, MapSize);

  uint32_t SeedWidth = 1;
  void *KernelParams[] = {(void *)&dSeeds, (void *)&SeedWidth}; 
  cuCheck(cuLaunchKernel(fnMain, GRID_X, GRID_Y, GRID_Z, BLOCK_X, BLOCK_Y, BLOCK_Z, 0,
                          NULL, &KernelParams[0], NULL)); 
  CUresult result = cuCtxSynchronize();
  if (result == CUDA_SUCCESS) 
    return NJOBS;
  else {
    WARNF("[Cuda] %d(%s)\n", static_cast<unsigned int>(result), _cudaGetErrorEnum(result)); 
    return 0;
  }
}

extern "C" uint64_t __declspec(dllexport) cuEvalSeeds(uint32_t SeedWidth) { 
  
  // uint8_t tmp[SEED_SIZE];
  // cuCheck(cuMemcpyDtoH(tmp, dSeeds + SEED_SIZE * 0, SEED_SIZE));
  // printf("[-] Host: seed "); printbytes(tmp+TRANSACTION_SIZE, TRANSACTION_SIZE); printf("\n");
  
  // clear signals
  // cuCheck(cuMemsetD32(dSignals, 0, 2));

  void *KernelParams[] = {(void *)&dSeeds, (void *)&SeedWidth}; 
  cuCheck(cuLaunchKernel(fnMain, GRID_X, GRID_Y, GRID_Z, BLOCK_X, BLOCK_Y, BLOCK_Z, 0,
                          NULL, &KernelParams[0], NULL)); 
  // cuCheck(cuLaunchKernel(fnMain, 1, 1, 1, 1, 1, 1, 0,
  //                         NULL, &KernelParams[0], NULL)); 
  cuCheck(cuCtxSynchronize());
  return NJOBS;
}


extern "C" uint64_t __declspec(dllexport) cuEvalTxn(uint32_t n_wrap) {
  // printf("[CUDA] cuEvalTxn() began\n");
  CUdeviceptr dtime;
  cuCheck(cuModuleGetGlobal(&dtime, nullptr, cuModule, "cuTimestamp"));
  struct timeval tv;    
  gettimeofday(&tv, NULL);     
  uint64_t tms = (tv.tv_sec * 1000ULL) + (tv.tv_usec / 1000);   
  uint64_t tns = tms * 1000000;
  cuCheck(cuMemcpyHtoD(dtime, &tns, sizeof(uint64_t)));

  CUdeviceptr dnwrap;
  // cuCheck(cuModuleGetGlobal(&dnwrap, nullptr, cuModule, "cuNwrap"));
  // cuCheck(cuMemcpyHtoD(dnwrap, &n_wrap, sizeof(n_wrap)));

  cuCheck(cuMemsetD8(dHnb, 0, NJOBS * sizeof(uint8_t)));
  uint32_t _tmp = 0;
  void *KernelParams[] = {(void *)&dSeeds, (void *)&_tmp}; 
  cuCheck(cuLaunchKernel(fnMain, GRID_X, GRID_Y, GRID_Z, BLOCK_X, BLOCK_Y, BLOCK_Z, 0,
                          NULL, &KernelParams[0], NULL)); 
  cuCheck(cuCtxSynchronize());
  // printf("[CUDA] cuEvalTxn() end\n");
  return NJOBS;
}


// extern "C" bool __declspec(dllexport) postGainDu(CUdeviceptr dSignals, const char *timeStr) { 
//   bool DuGain = false;
//   /* obtain hSignals for this stream */
//   uint64_t SigsLen;
//   cuCheck(cuMemcpyDtoH(&SigsLen, dSignals, sizeof(uint64_t)));
//   // printf("len = %d\n", SigsLen);
//   if (!SigsLen) return false;

//   cuCheck(cuMemcpyDtoH(hSignals, dSignals + sizeof(uint64_t),
//                        sizeof(uint64_t) * SigsLen));
//   for (auto SigIdx = 0; SigIdx < SigsLen; SigIdx++) {
//     const uint64_t Signal = hSignals[SigIdx];
//     const uint64_t BugHash = Signal & 0xffffffffff;
//     if (BugSet.find(BugHash) == BugSet.end()) {
//       BugSet.insert(BugHash);
//       const uint32_t ThreadID = (Signal >> 40) & 0xffffff;
//       const uint32_t PC = BugHash & 0xffffffff;
//       const uint32_t Bug = (BugHash >> 32) & 0xff;
//       OKF("%sThread#%d found %s at %x\n", timeStr, ThreadID, BugToTag(SigByte(Bug)).c_str(), PC);
//       DuGain = true;
//     }
//   }
//   return DuGain;
// }

extern "C" bool __declspec(dllexport) getCudaExecRes(uint64_t *pcov, uint64_t *pbug) {
  // retrive the results
  cuCheck(cuModuleGetFunction(&fnEval, cuModule, "updateBits"));  
  void *KernelParams[] = {(void *)&dSig};
  cuCheck(cuLaunchKernel(fnEval, /*grid_dim=*/1, 1, 1,
                        /*block dim=*/MAP_SIZE/8, 1, 1, /*shared_mem=*/0,
                        /*stream=*/0, /*arguments=*/KernelParams, 0));
  cuCheck(cuCtxSynchronize());
  // cuCheck(cuMemcpyDtoH(hnbs, dHnb, sizeof(uint8_t) * NJOBS));

  uint64_t signal = 0;
  cuCheck(cuMemcpyDtoH(&signal, dSig, sizeof(uint64_t)));

#ifdef DEBUG_FLAG
  printf("Driver. obtained SIGNAL from kernel => %llx\n", signal);
#endif

  *pbug = (uint32_t)(signal & 0xffffffff);
  cuCheck(cuMemcpyDtoH(hnbs, dHnb, sizeof(uint8_t) * NJOBS));
  return signal != 0;
}

// extern "C" uint64_t __declspec(dllexport) postGainCov() {
//   cuCheck(cuMemsetD8(dHnb, 0, NJOBS * sizeof(uint8_t)));
//   // retrive the results
//   cuCheck(cuModuleGetFunction(&fnEval, cuModule, "updateBits"));  
//   void *KernelParams[] = {(void *)&dSig, (void*)&dHnb};
//   cuCheck(cuLaunchKernel(fnEval, /*grid_dim=*/1, 1, 1,
//                         /*block dim=*/MAP_SIZE/8, 1, 1, /*shared_mem=*/0,
//                         /*stream=*/0, /*arguments=*/KernelParams, 0));
//   cuCheck(cuCtxSynchronize());
//   // cuCheck(cuMemcpyDtoH(hnbs, dHnb, sizeof(uint8_t) * NJOBS));

//   uint64_t  = 0;
//   cuCheck(cuMemcpyDtoH(&coverage, dSig, sizeof(uint64_t)));

//   // cuCheck(cuMemcpyDtoH(bugyy, dSignals, sizeof(uint64_t)));

//   // if (Coverage < *coverage) {
//   //   Coverage = *coverage;
//   //   return true;

//   // } else {
//   //   return false;
//   // }
//   return coverage;
// }

// extern "C" uint64_t __declspec(dllexport) obtainCov() {
//   cuCheck(cuMemcpyDtoH(&Coverage, dCov, sizeof(uint64_t)));
//   return Coverage;
// }

extern "C" uint8_t __declspec(dllexport) isCudaInteresting(uint32_t tid) {     
  return hnbs[tid];
}

extern "C" uint8_t __declspec(dllexport) gainCov(uint32_t tid, uint8_t *calldata) {   
  uint8_t hnb = hnbs[tid];
  if (hnb != EXECNONE) {
    // only calldata
    cuCheck(cuMemcpyDtoH(calldata, dSeeds + SEED_SIZE * tid, SEED_SIZE));
  }
  
  return hnb;
}

// extern "C" bool __declspec(dllexport) gainBug(uint32_t tid) {
//   // bug-based
//   // CUdeviceptr dSignals;
//   // cuCheck(cuModuleGetGlobal(&dSignals, nullptr, cuModule, "__signals"));
//   // cuCheck(cuMemcpyHtoD(dArgMap, &MapSize, sizeof(uint8_t))); // (uint8_t)MapSize
//   return false;
// }

// extern "C" void __declspec(dllexport) cuDumpStorage(uint32_t threadId = 0) {
  // FIXME: dump the L1&L2 snapshot
  // uint8_t slotCnt = 0;
  // slot_t *storage = nullptr;
  // printf("---------------- Dumping Thread #%zu -----------------\n", threadId);

  // // root storage
  // CUdeviceptr dL2SnapLen, dL2Snap;
  // cuCheck(cuModuleGetGlobal(&dL2SnapLen, nullptr, cuModule, L2SNAP_LEN));
  // cuCheck(cuModuleGetGlobal(&dL2Snap, nullptr, cuModule, L2SNAP));
  // cuCheck(cuMemcpyDtoH(&slotCnt, dL2SnapLen, sizeof(uint8_t) * NJOBS));
  // storage = new slot_t[slotCnt];
  // cuCheck(cuMemcpyDtoH(storage, dL2Snap, sizeof(slot_t) * slotCnt));
  // printf("=>Root storage #%d:\n", slotCnt); 
  // for (auto i = 0; i < slotCnt; i++) {
  //   printf("----------------------------------\n"); 
  //   printbytes((uint8_t *)&storage[i].key[0], 32);
  //   printbytes((uint8_t *)&storage[i].val[0], 32);
  // }
  // delete storage; 

  // // runtime storage
  // CUdeviceptr dL2SnapLenList, dL2Snaps;
  // cuCheck(cuModuleGetGlobal(&dL2SnapLenList, nullptr, cuModule, L2SNAP_LENS));
  // cuCheck(cuModuleGetGlobal(&dL2Snaps, nullptr, cuModule, L2SNAPS));
  // cuCheck(cuMemcpyDtoH(&slotCnt, dL2SnapLenList + threadId, sizeof(uint8_t)));
  // printf("=>L2 storage #%d:\n", slotCnt); 

  // storage = new slot_t[slotCnt];
  // // OKF("Loaded a storage including %d slots", slotCnt);
  // if (slotCnt > ONE_SNAP_LEN) printf("Increase the SnapshotSize. Used slots %d/%d.", slotCnt, ONE_SNAP_LEN);
  // cuCheck(cuMemcpyDtoH(storage, dL2Snaps + sizeof(slot_t) * ONE_SNAP_LEN * threadId, sizeof(slot_t) * slotCnt));
  // for (auto i = 0; i < slotCnt; i++) {
  //   printf("----------------------------------\n"); 
  //   printbytes((uint8_t *)&storage[i].key[0], 32);
  //   printbytes((uint8_t *)&storage[i].val[0], 32);
  // }
  // delete storage; 
// }


extern "C" void __declspec(dllexport) cuLoadStorage(uint8_t *src, const uint32_t slotCnt, uint32_t state_id) {  
  if (slotCnt > ONE_SNAP_LEN) {
    WARNF("loaded a too large storage with %d\n", slotCnt);
  }
  uint32_t state_pos = state_id;
  // if (storage_pos_map.find(state_id) != storage_pos_map.end()) {
  //   // the storage had been loaded before
  //   return;
  // } else {
  //   state_pos = storage_pos_map.size();
  //   storage_pos_map[state_id] = state_pos;
  // }
  // printf("candaites size = %d\n", storage_pos_map.size());

  uint8_t slotCnt8 = (uint8_t)slotCnt;

#ifdef DEBUG_FLAG
  printf("---------------- loading storage --------------\n"); 
  for (auto i = 0; i < slotCnt8; i++) {
    printf("key: "); printbytes(src + 64 * i, 32);
    printf("val: "); printbytes(src + 64 * i + 32, 32);
    printf("----------------------------------\n"); 
  }
#endif
  CUdeviceptr dL2SnapLenVec, dL2SnapVec;
  cuCheck(cuModuleGetGlobal(&dL2SnapLenVec, nullptr, cuModule, L2SNAP_LEN_VEC));
  cuCheck(cuMemcpyHtoD(dL2SnapLenVec + sizeof(uint8_t) * state_pos, &slotCnt8, sizeof(uint8_t)));
  if (slotCnt8 > 0) {
    cuCheck(cuModuleGetGlobal(&dL2SnapVec, nullptr, cuModule, L2SNAP_VEC));
    cuCheck(cuMemcpyHtoD(dL2SnapVec + ONE_SNAP_LEN * sizeof(slot_t) * state_pos, src, sizeof(slot_t) * slotCnt8));
  }
}