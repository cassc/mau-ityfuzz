#include <cstdio>

#include <cuda.h>
#include <cuda_runtime_api.h>

// static std::set<uint64_t> BugSet;
// static uint64_t TotalCrashes = 0;

// static uint64_t *hSignals = nullptr;  // host pinned
// static bool DuGains[NJOBS];
// static uint64_t Coverage = 0;
// static uint32_t BytecodeSize = 0;



static CUfunction fnMain, fnDeploy, fnEval = NULL;
static CUdeviceptr dEvmCode, dEvmCodeSize, dnothing = 0;
static CUdeviceptr dCov, dHnb = 0;

extern "C"  uint32_t cuVersion();

extern "C" void InitCudaCtx(const int Dev, const char *pathToKernel);

extern "C" void DestroyCuda();

// extern "C" bool setEVMEnv(uint8_t *From, const uint64_t Timestamp, const uint64_t Blocknum);
extern "C" bool setEVMEnv(uint8_t *From, uint8_t *To, uint8_t *Timestamp, uint8_t*Blocknum, uint8_t *Callvalue);

extern "C" void cuMallocAll();

extern "C" void cuLoadConstBytes(uint32_t idx, uint8_t *constant, uint8_t size);

extern "C" void cuExtSeeds(uint8_t *rawSeed, uint32_t size);

extern "C" void cuMutate(uint32_t calldasize, 
    uint8_t *argTypes, const uint32_t argTypesLen);

extern "C" bool cuDeployTx(uint64_t callvalue,
                           uint8_t *Buff, uint32_t ArgSize);

/// establish the parallel-data dseed
extern "C" bool 
cuDataCpy(const CUdeviceptr dev, uint64_t callvalue, uint8_t *buff, uint32_t size);

/// run the runtimer
// extern "C" bool 
// cuRunTxsInGroup(CUmodule cudaModule, CUdeviceptr dSeeds, 
//                 uint64_t callvalue, uint8_t *Buff, uint32_t ArgSize);

/// run the runtimer
extern "C" uint64_t cuRunTxs(uint8_t *ArgTypeMap, uint32_t MapSize);
  
extern "C" uint64_t cuEvalSeeds(uint32_t SeedWidth);
extern "C" uint64_t cuEvalTxn(uint32_t size);

extern "C" bool postGainDu(CUdeviceptr dSignals, const char *timeStr);

// extern "C" uint64_t postGainCov();
// extern "C" uint64_t postGainCov();

extern "C" bool getCudaExecRes(uint64_t *pcov, uint64_t *pbug);

// extern "C" uint64_t obtainCov();

extern "C" uint8_t gainCov(uint32_t tid, uint8_t *rawSeed);

// extern "C" bool gainBug(uint32_t tid);

extern "C" void cuDumpStorage(uint32_t threadId=0); 

extern "C" void cuLoadStorage(uint8_t *src, uint32_t slotCnt);


extern "C" uint32_t cuGetThreads() { return NJOBS; }
extern "C" size_t cuGetTxSize() { return TRANSACTION_SIZE; }