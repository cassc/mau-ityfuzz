#include <assert.h>
#include <string.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>
#include <set>

#include <cuda.h>
#include "../src/config.h"

typedef uint8_t u8;
typedef std::vector<uint8_t> Unit;
typedef std::vector<Unit> UnitVector;
struct slot_t {
  uint64_t key[4];
  uint64_t val[4];
};

uint32_t bytesfromhex_bigEndian(u8 bytes[], const u8* rawdata, const size_t size) {
  const char *hexdata = (char*)rawdata; 

  uint32_t offset = 0;
  if (hexdata[0] == '0' && (hexdata[1] == 'x' || hexdata[1] == 'X')) offset = 2;

  for (uint32_t i = 0; i < size; i++) {
    std::sscanf(hexdata + offset + i * 2, "%02hhx", &bytes[size - 1 - i]);
  }
  return (size - offset) / 2;
}

uint32_t bytesfromhex_littleEndian(u8 bytes[], const u8* rawdata, const size_t size) {
  const char *hexdata = (char*)rawdata; 

  uint32_t offset = 0;
  if (hexdata[0] == '0' && (hexdata[1] == 'x' || hexdata[1] == 'X')) offset = 2;

  for (uint32_t i = 0; i < size; i++) {
    std::sscanf(hexdata + offset + i * 2, "%02hhx", &bytes[i]);
  }
  return (size - offset) / 2;
}


uint32_t bytesfromhex(u8 bytes[], const std::string rawdata) {
  uint32_t size = (uint32_t)rawdata.size();
  assert(size % 2 == 0);
  const char *hexdata = rawdata.c_str(); 

  uint32_t offset = 0;
  if (hexdata[0] == '0' && (hexdata[1] == 'x' || hexdata[1] == 'X')) offset = 2;

  for (uint32_t i = 0; i < size; i++) {
    std::sscanf(hexdata + offset + i * 2, "%02hhx", &bytes[i]);
  }
  return (size - offset) / 2;
}

void printbytes(u8 bytes[], uint32_t _size) {
  for (uint32_t i = 0; i < _size; ++i) {
    printf("%02hhx", bytes[i]);
  }
}


#define checkCudaErrors(err) do { \
  if (err != CUDA_SUCCESS) { \
  const char *info; \
  cuGetErrorString(err, &info); \
  const char *errName;  \
  cuGetErrorName(err, &errName);  \
  std::cerr << "[ERROR-" << err << "] " << errName << ": " << info << std::endl; \
  std::cerr << "We are using CUDA device #" << getenv("CUDA_DEVICE_ID") << ".\n"; \
  std::cerr << "If the remaining GPU resource is insufficient. Please use another GPU device by " \
                "executing the following command:\n\n   export "  \
                "CUDA_DEVICE_ID=<Number> # query an avaliable GPU by executing `nvidia-smi`\n"; \
  std::cerr << "    Stop location :" << __FUNCTION__ <<  "(), " << __LINE__ << "\n"; \
  abort(); \
  }\
} while (0)


void dumpStorage(CUmodule& cudaModule) {
  CUdeviceptr dMasterSnapLen, dMasterSnap, dL2SnapLenList, dL2Snaps;

  checkCudaErrors(cuModuleGetGlobal(&dMasterSnapLen, NULL, cudaModule, L3SNAP_LEN));
  checkCudaErrors(cuModuleGetGlobal(&dMasterSnap, NULL, cudaModule, L3SNAP));

  uint8_t slotCnt;
  checkCudaErrors(cuMemcpyDtoH(&slotCnt, dMasterSnapLen, sizeof(uint8_t)));

  slot_t *storage = new slot_t[slotCnt];
  checkCudaErrors(cuMemcpyDtoH(storage, dMasterSnap, sizeof(slot_t) * slotCnt));
  printf("Loaded a storage including %d slots", slotCnt);
  for (auto i = 0; i < slotCnt; i++) {
    printf("---------\n"); 
    printbytes((uint8_t *)&storage[i].key[0], 32); printf(" ");
    printbytes((uint8_t *)&storage[i].val[0], 32);
  }
  delete storage; 
}

void dumpStorage(CUmodule& cudaModule, size_t threadId) {
  printf("------------------------ Thread #%d -------------------------\n", threadId);
  CUdeviceptr dL2SnapLenList, dL2Snaps;
  checkCudaErrors(cuModuleGetGlobal(&dL2SnapLenList, nullptr, cudaModule, L2SNAP_LENS));
  checkCudaErrors(cuModuleGetGlobal(&dL2Snaps, nullptr, cudaModule, L2SNAPS));

  uint8_t slotCnt;
  checkCudaErrors(cuMemcpyDtoH(&slotCnt, dL2SnapLenList + threadId, sizeof(uint8_t)));
  slot_t *storage = new slot_t[slotCnt];

  // OKF("Loaded a storage including %d slots", slotCnt);
  if (slotCnt > ONE_SNAP_LEN) printf("Increase the SnapshotSize. Used slots %d/%d.", slotCnt, ONE_SNAP_LEN);
  checkCudaErrors(cuMemcpyDtoH(storage, dL2Snaps + sizeof(slot_t) * ONE_SNAP_LEN * threadId, sizeof(slot_t) * slotCnt));
  for (auto i = 0; i < slotCnt; i++) {
    printf("----------------------------------\n"); 
    printbytes((uint8_t *)&storage[i].key[0], 32);
    printf("\n");
    printbytes((uint8_t *)&storage[i].val[0], 32);
  }
  printf("\n"); 
  delete storage; 
}

bool ConsistentStorage(CUmodule& cudaModule, size_t threadSize) {
  if (threadSize < 1) return true;
  CUdeviceptr dL2SnapLenList, dL2Snaps;
  checkCudaErrors(cuModuleGetGlobal(&dL2SnapLenList, nullptr, cudaModule, L2SNAP_LENS));
  checkCudaErrors(cuModuleGetGlobal(&dL2Snaps, nullptr, cudaModule, L2SNAPS));
  uint8_t SlotCnt;
  checkCudaErrors(cuMemcpyDtoH(&SlotCnt, dL2SnapLenList + 0, sizeof(uint8_t)));
  if (SlotCnt > ONE_SNAP_LEN) 
    printf("Increase the SnapshotSize. Used slots %d/%d.", SlotCnt, ONE_SNAP_LEN);
  slot_t *Storage = new slot_t[SlotCnt];
  checkCudaErrors(cuMemcpyDtoH(Storage, dL2Snaps, sizeof(slot_t) * SlotCnt));

  slot_t *tmpStorage = new slot_t[SlotCnt];
  size_t threadId = 1;
  for (; threadId < threadSize; threadId++){
    uint8_t tmpCnt;
    checkCudaErrors(cuMemcpyDtoH(&tmpCnt, dL2SnapLenList + threadId, sizeof(uint8_t)));
    if (tmpCnt != SlotCnt) break;

    checkCudaErrors(cuMemcpyDtoH(tmpStorage, 
        dL2Snaps + sizeof(slot_t) * ONE_SNAP_LEN * threadId, sizeof(slot_t) * SlotCnt));
    
    if (memcmp(Storage, tmpStorage, sizeof(slot_t) * SlotCnt) != 0)
      break;
  }
  delete Storage; 
  delete tmpStorage;
  return threadId == threadSize ? true : false;
}

void debug(CUcontext ctx, CUdevice device){
  checkCudaErrors(cuCtxCreate(&ctx, 0, device));  
}

/// main - Program entry point
int main(int argc, char **argv) {
  static std::set<uint64_t> BugSet;

  CUdevice device;
  CUmodule cudaModule;
  CUcontext context;
  int devCount;
  // CUDA initialization
  checkCudaErrors(cuInit(0));
  checkCudaErrors(cuDeviceGetCount(&devCount));
  int cuda_device = 4; // default value
  const char *content = getenv("CUDA_DEVICE_ID");
  if (content != NULL) {
    cuda_device = *content - '0';
  }
  
  if (strlen(content) != 1 || cuda_device < 0 || cuda_device >= devCount) {
    std::cerr << "There are #" << devCount
        << " CUDA devices, but you are using an invalid one.\nCUDA_DEVICE_ID:"
        << content << "\n";
    exit(-1);
  }
  checkCudaErrors(cuDeviceGet(&device, cuda_device));
  // CUDA Environment
  // checkCudaErrors(cuCtxCreate(&context, 0, device));
  debug(context, device);
  checkCudaErrors(cuCtxSetLimit(CU_LIMIT_STACK_SIZE, 8192));
  // cuCtxGetLimit(&d, CU_LIMIT_STACK_SIZE);
  // std::cerr << "CU_LIMIT_STACK_SIZE = " << d << "\n";
  // checkCudaErrors(cuCtxSetLimit(CU_LIMIT_PRINTF_FIFO_SIZE, 11));
  // cuCtxGetLimit(&d, CU_LIMIT_PRINTF_FIFO_SIZE);
  // std::cerr << "CU_LIMIT_PRINTF_FIFO_SIZE = " << d << "\n";
  // checkCudaErrors(cuCtxSetLimit(CU_LIMIT_MALLOC_HEAP_SIZE, 4096));
  // cuCtxGetLimit(&d, CU_LIMIT_MALLOC_HEAP_SIZE);
  // std::cerr << "CU_LIMIT_MALLOC_HEAP_SIZE = " << d << "\n";
  // // checkCudaErrors(cuCtxSetLimit(CU_LIMIT_DEV_RUNTIME_PENDING_LAUNCH_COUNT, 101));
  // cuCtxGetLimit(&d, CU_LIMIT_DEV_RUNTIME_PENDING_LAUNCH_COUNT);
  // std::cerr << "CU_LIMIT_DEV_RUNTIME_PENDING_LAUNCH_COUNT = " << d << "\n";
  // checkCudaErrors(cuCtxSetLimit(CU_LIMIT_DEV_RUNTIME_SYNC_DEPTH, 20));
  // cuCtxGetLimit(&d, CU_LIMIT_DEV_RUNTIME_SYNC_DEPTH);
  // std::cerr << "CU_LIMIT_DEV_RUNTIME_SYNC_DEPTHT = " << d << "\n";
  
  std::string pathToKernel = argv[1];
  std::string pathToSeed = argv[2];
  size_t hostReturndataSize = std::stoi(argv[3]);
  std::cerr << "Ret Size = " << hostReturndataSize << "\n";


  std::ifstream _(pathToKernel);
  if (!_.good()) {
    printf("[-] Cannot find %s", pathToKernel.c_str());
    exit(-1);
  }
  checkCudaErrors(cuModuleLoad(&cudaModule, pathToKernel.c_str()));
  
  CUfunction fnMain, fnDeploy;
  checkCudaErrors(cuModuleGetFunction(&fnDeploy, cudaModule, CUDA_KERNEL_DEPLOYER_FUNC));
  checkCudaErrors(cuModuleGetFunction(&fnMain, cudaModule, CUDA_KERNEL_FUNC));

  printf("[+] Obtained the seed from %s\n", pathToSeed.c_str());
  std::ifstream Infile(pathToSeed, std::ios::in);
  if (!Infile) printf("Fail to read: %s\n", pathToSeed.c_str());
  uint8_t *Buff = new uint8_t[TRANSACTION_Bytes];
  uint64_t Callvalue;
  std::string InputHex;
  UnitVector Us;
  while (Infile >> Callvalue >> InputHex) {
    printf("CallValue = %lld, InputHex = %s\n", Callvalue, InputHex.c_str());
    uint64_t CalldataSize = InputHex.size() / 2;
    uint64_t InputSize = 8 + 4 + CalldataSize;
    if (InputSize >= TRANSACTION_Bytes) {
      printf("[-] the Calldata is too long: %s\n", pathToSeed.c_str());
      exit(-1);
    }
    *(uint64_t*)Buff = Callvalue;
    *(uint32_t*)(Buff + 8) = CalldataSize;
    bytesfromhex(Buff + 12, InputHex);
    Us.push_back({Buff, Buff + InputSize});
  }
  Infile.close();
  delete Buff;
 
  // Device data
  CUdeviceptr dSeeds, devBufferRet, dSignals, dnothing;
  checkCudaErrors(cuMemAlloc(&dSeeds, TRANSACTION_Bytes*NJOBS));
  checkCudaErrors(cuModuleGetGlobal(&devBufferRet, nullptr, cudaModule, DEBUG_BUFFER));
  // checkCudaErrors(cuModuleGetGlobal(&dSignals, nullptr, cudaModule, SIGNALS_REG));
  checkCudaErrors(cuMemAlloc(&dnothing, 1));

  CUdeviceptr dL2SnapLenList;
  checkCudaErrors(cuModuleGetGlobal(&dL2SnapLenList, nullptr, cudaModule, L2SNAP_LENS));
  cuMemsetD8(dL2SnapLenList, 0, sizeof(uint8_t) * NJOBS);  

  // setup the bitmap
  CUdeviceptr d_bitmaps;
  checkCudaErrors(cuModuleGetGlobal(&d_bitmaps, nullptr, cudaModule, BITMAPS_REG));
  CUdeviceptr d_one, dOneBitmap;
  for (auto i = 0; i < NJOBS; ++i) {
    checkCudaErrors(cuMemAlloc(&d_one, MAP_SIZE));
    checkCudaErrors(cuMemsetD8(d_one, 0, MAP_SIZE));  
    dOneBitmap = d_bitmaps + i * sizeof(uint8_t *);
    checkCudaErrors(cuMemcpyHtoD(dOneBitmap, &d_one, 8));
  }

  uint32_t tidOffset = 0;
  cuMemsetD8(dL2SnapLenList, 0, sizeof(uint8_t)); 
  uint64_t hSignals[SIGNAL_VEC_LEN];

  CUdeviceptr dEvmCode, dCodeSize;
  checkCudaErrors(cuModuleGetGlobal(&dEvmCode, nullptr, cudaModule, EVMCODE_REG));
  checkCudaErrors(cuModuleGetGlobal(&dCodeSize, nullptr, cudaModule, EVMCODESIZE_REG));

  uint32_t BytecodeSize;
  checkCudaErrors(cuMemcpyDtoH(&BytecodeSize, dCodeSize, sizeof(BytecodeSize)));

  // run the deployer
  printf("Deployer Input: %u Bytes => ", Us[0].size());
  printbytes(Us[0].data(), Us[0].size()); printf("\n");

  checkCudaErrors(cuMemcpyHtoD(dSeeds, Us[0].data(), 12)); // callvalue
  checkCudaErrors(cuMemcpyHtoD(dEvmCode + BytecodeSize, Us[0].data() + 12, Us[0].size() - 12)); // callvalue
  uint32_t CodeSize = BytecodeSize + Us[0].size() - 12;
  checkCudaErrors(cuMemcpyHtoD(dCodeSize, &CodeSize, 4));
  printf("Deployer: BytecodeSize= %uB; CodeSize = %uB => ", BytecodeSize, CodeSize);
  
  CUdeviceptr dCaller, dOrigin;
  uint8_t tmp[40];
  std::string CallerStr = "0x24cd2edba056b7c654a50e8201b619d4f624fdda";
  if (cuModuleGetGlobal(&dCaller, nullptr, cudaModule, CALLER_REF) == CUDA_SUCCESS) {
    bytesfromhex_bigEndian(&tmp[0], (uint8_t *)CallerStr.c_str(), 20);
    checkCudaErrors(cuMemcpyHtoD(dCaller, &tmp[0], 20));
  }
  if (cuModuleGetGlobal(&dOrigin, nullptr, cudaModule, ORIGIN_REF) == CUDA_SUCCESS) {
    bytesfromhex_bigEndian(&tmp[0], (uint8_t *)CallerStr.c_str(), 20);
    checkCudaErrors(cuMemcpyHtoD(dOrigin, &tmp[0], 20));
  }

  void *DeployerParams[] = {(void *)&dSeeds, (void *)&dnothing}; 
  checkCudaErrors(cuLaunchKernel(fnDeploy, 1, 1, 1, 1, 1, 1, 0,
                                 NULL, &DeployerParams[0], NULL)); 
  checkCudaErrors(cuCtxSynchronize());  
  dumpStorage(cudaModule);

  // dry run one seed
  cuMemsetD8(dSignals, 0, sizeof(uint64_t));  
  for(auto itr = std::next(Us.begin()); itr != Us.end(); itr++) {
    Unit &U = *itr;

    printf("Runtimer Input: %u Bytes => ", U.size());
    printbytes(U.data(), U.size()); printf("\n");
    
    for (auto tid = 0; tid < NJOBS; tid++)
      checkCudaErrors(cuMemcpyHtoD(dSeeds + TRANSACTION_Bytes * tid, U.data(), U.size()));  

    void *KernelParams[] = {(void *)&dSeeds, (void *)&dnothing,             
                            (void *)&dnothing, (void *)&tidOffset};         
    checkCudaErrors(cuLaunchKernel(fnMain, GRID_X, GRID_Y, GRID_Z, 
                                   BLOCK_X, BLOCK_Y, BLOCK_Z, 0,
                                   NULL, &KernelParams[0], NULL)); 
    checkCudaErrors(cuCtxSynchronize());  
  }
  /* obtain hSignals for this stream */                                    
  uint64_t SigsLen = 0;
  checkCudaErrors(cuMemcpyDtoH(&SigsLen, dSignals, sizeof(uint64_t))); 
  if (SigsLen > 0) {
    checkCudaErrors(cuMemcpyDtoH(hSignals, 
        dSignals + sizeof(uint64_t), sizeof(uint64_t) * SigsLen));
    for (auto SigIdx = 0; SigIdx < SigsLen; SigIdx++) {
      const uint64_t Signal = hSignals[SigIdx];
      const uint64_t BugHash = Signal & 0xffffffffff;        
      if (BugSet.find(BugHash) == BugSet.end()) {
        BugSet.insert(BugHash);
        const uint32_t ThreadID = (Signal >> 40) & 0xffffff;
        const uint32_t PC = BugHash & 0xffffffff;
        const uint32_t Bug = (BugHash >> 32) & 0xff; 
        printf("[+] Unique Crash Found: %06lx-%02x-%08lx", ThreadID, Bug, PC);  
      }
    }
  }
  
  if (ConsistentStorage(cudaModule, NJOBS)) {
    // Retrieve device data
    u8 hostRes[hostReturndataSize];
    checkCudaErrors(cuMemcpyDtoH(&hostRes[0], devBufferRet, 
        sizeof(char) * hostReturndataSize));
    printf("Output: \n");
    printbytes(&hostRes[0], hostReturndataSize);
  }
  else {
    dumpStorage(cudaModule, 0);
    dumpStorage(cudaModule, 1);
    printf("Inconsistent execution\n");
    exit(-1);

  }
  
  // Clean-up
  cuMemFree(dSeeds);
  cuMemFree(dnothing);
  cuModuleUnload(cudaModule);
  cuCtxDestroy(context);
  
  return 0;
}