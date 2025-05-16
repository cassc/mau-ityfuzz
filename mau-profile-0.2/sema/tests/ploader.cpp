#include <assert.h>
#include <string.h>

#include <cstdio>
#include <fstream>
#include <iostream>

#include <cuda.h>
#include "../src/config.h"

static inline uint8_t has_new_bits(uint8_t* virgin_map, uint8_t* trace_bits) {

#ifdef WORD_SIZE_64
  uint64_t* current = (uint64_t*)trace_bits;
  uint64_t* virgin  = (uint64_t*)virgin_map;

  uint32_t  i = (MAP_SIZE >> 3);

#else

  uint32_t* current = (uint32_t*)trace_bits;
  uint32_t* virgin  = (uint32_t*)virgin_map;

  uint32_t  i = (MAP_SIZE >> 2);

#endif /* ^WORD_SIZE_64 */

  uint8_t   ret = 0;

  while (i--) {

    /* Optimize for (*current & *virgin) == 0 - i.e., no bits in current bitmap
       that have not been already cleared from the virgin map - since this will
       almost always be the case. */

    if (*current && *current & *virgin) {

      if (ret < 2) {

        uint8_t* cur = (uint8_t*)current;
        uint8_t* vir = (uint8_t*)virgin;

        /* Looks like we have not found any new bytes yet; see if any non-zero
           bytes in current[] are pristine in virgin[]. */

#ifdef WORD_SIZE_64
        if ((cur[0] && vir[0] == 0xff) || (cur[1] && vir[1] == 0xff) ||
            (cur[2] && vir[2] == 0xff) || (cur[3] && vir[3] == 0xff) ||
            (cur[4] && vir[4] == 0xff) || (cur[5] && vir[5] == 0xff) ||
            (cur[6] && vir[6] == 0xff) || (cur[7] && vir[7] == 0xff)) ret = 2;
        else ret = 1;
#else
        if ((cur[0] && vir[0] == 0xff) || (cur[1] && vir[1] == 0xff) ||
            (cur[2] && vir[2] == 0xff) || (cur[3] && vir[3] == 0xff)) ret = 2;
        else ret = 1;

#endif /* ^WORD_SIZE_64 */

      }
      *virgin &= ~*current;
    }
    current++;
    virgin++;
  }
  return ret;
}

uint32_t bytesfromhex(uint8_t bytes[], const std::string rawdata) {
  uint32_t size = (uint32_t)rawdata.size();
  assert(size % 2 == 0);
  const char *hexdata = rawdata.c_str(); 

  uint32_t offset = 0;
  if (hexdata[0] == '0' && (hexdata[1] == 'x' || hexdata[1] == 'X')) offset = 2;

  for (uint32_t i = 0; i < size; i++) {
    std::sscanf(hexdata + offset + i * 2, "%02hhx", &bytes[i]);
  }

  // padding to TRANSACTION_SIZE
  for (uint32_t i = size; i < TRANSACTION_SIZE; i++) {
    bytes[i] = 0;
  }
    
  return (size - offset) / 2;
}

void printbytes(uint8_t bytes[], uint32_t _size) {
  for (uint32_t i = 0; i < _size; ++i) {
    printf("%02hhx", bytes[i]);
  }
}

#define checkCuda(err) do { \
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
  printf("\n    Stop location : %s(), %s:%u\n\n", __FUNCTION__, __FILE__, __LINE__); \
  abort(); \
  }\
} while (0)


/// main - Program entry point
int main(int argc, char **argv) {
  const int ROUND = 1;

  const unsigned blockSizeX = 8;  
  const unsigned blockSizeY = 1;
  const unsigned blockSizeZ = 1;
  const unsigned gridSizeX = 4096*32;
  const unsigned gridSizeY = 1;
  const unsigned gridSizeZ = 1;

  const int nStreams = 4;
  const int txsLength = (argc - 2) / 2;
  
  const int n =  nStreams * blockSizeX * gridSizeX; // jobs
  
  const int bytes = n * TRANSACTION_SIZE * txsLength * sizeof(uint8_t);
  const int bitmapBytes = n * MAP_SIZE * sizeof(uint8_t);

  const int streamBytes = bytes / nStreams;
  const int streamBitmapBytes = bitmapBytes / nStreams;

  CUdevice device;
  CUmodule cudaModule;
  CUcontext context;
  CUfunction function;
  int devCount;
  // CUDA initialization
  checkCuda(cuInit(0));
  checkCuda(cuDeviceGetCount(&devCount));

  int cuda_device = 0;
  const char *content = getenv("CUDA_DEVICE_ID");
  if (content != NULL) {
    cuda_device = *content - '0';
  }
  if (cuda_device < 0 || cuda_device >= devCount) {
    std::cerr
        << "There are #" << devCount
        << " CUDA devices, but you are using an invalid one.\nCUDA_DEVICE_ID:"
        << content << "\n";
    exit(-1);
  }

  checkCuda(cuDeviceGet(&device, cuda_device));

  checkCuda(cuCtxCreate(&context, 0, device));
  std::string path_to_kernel = argv[1];
  size_t hostReturndataSize = 32;

  uint8_t *host_bitmap;
  checkCuda( cuMemAllocHost((void**)&host_bitmap, bitmapBytes) ); // host pinned

  uint8_t *hostMem;
  checkCuda( cuMemAllocHost((void**)&hostMem, bytes) );      // host pinned

  uint8_t *pmem = nullptr;
  uint32_t oneCalldataSize = 0;

  uint32_t hostMemSize = 0;
  for (int i = 2; i < argc; i+=2) {
    pmem = hostMem + hostMemSize;
    *(uint64_t*)(pmem) = std::stoul(argv[i]); 
    oneCalldataSize = bytesfromhex(pmem + 12, argv[i + 1]);   // read & extend 
    *(uint32_t*)(pmem + 8) = oneCalldataSize;
    hostMemSize += TRANSACTION_SIZE; // (8 + 4 + oneCalldataSize);
  }

  // extend hostMem to all threads
  for (int i = 1; i < n; ++i) {
    uint32_t windowSize = TRANSACTION_SIZE * txsLength * sizeof(uint8_t);
    memcpy(hostMem+windowSize*i, hostMem, windowSize);
  }

  std::ifstream _t(path_to_kernel);
  if (!_t.is_open()) {
    std::cerr << path_to_kernel << " not found\n";
    return -1;
  }
  std::string kernel_content((std::istreambuf_iterator<char>(_t)),
                  std::istreambuf_iterator<char>());
  checkCuda(cuModuleLoadDataEx(&cudaModule, kernel_content.c_str(), 0, 0, 0));
  checkCuda(cuModuleGetFunction(&function, cudaModule, CUDA_KERNEL_FUNC));
  checkCuda(cuCtxSetLimit(CU_LIMIT_STACK_SIZE, 4096));

  size_t d;
  checkCuda(cuCtxSetLimit(CU_LIMIT_MALLOC_HEAP_SIZE, 257698037));
  cuCtxGetLimit(&d, CU_LIMIT_MALLOC_HEAP_SIZE);
  std::cerr << "CU_LIMIT_MALLOC_HEAP_SIZE = " << d << "\n";

  // Device data
  CUdeviceptr devBufferMem, devBufferRet, devBuffer_bitmap;
  checkCuda(cuMemAlloc(&devBufferMem, bytes));
  checkCuda(cuMemAlloc(&devBuffer_bitmap, bitmapBytes));
  checkCuda(cuMemAlloc(&devBufferRet, sizeof(uint8_t) * hostReturndataSize));
  
  CUevent startEvent, stopEvent;
  checkCuda( cuEventCreate(&startEvent, 0) );
  checkCuda( cuEventCreate(&stopEvent, 0) );
  float ms;
  
  CUstream *streams = (CUstream *)malloc(nStreams * sizeof(CUstream));
  for (int i = 0; i < nStreams; i++) {
    checkCuda(cuStreamCreate(&(streams[i]), 0));
  }  

  size_t memfree, memtotal;
  cuMemGetInfo(&memfree, &memtotal);
  std::cerr << "free mem = " << memfree << " B, total mem = " << memtotal << "B\n";


  // sequential in groups
  checkCuda( cuEventRecord(startEvent, 0) );
  for (int _ = 0; _ < ROUND; ++_) {     
    cuMemsetD8(devBufferMem, 'A', bytes);
    cuMemsetD8(devBuffer_bitmap, '\x00', bitmapBytes);
    for (size_t i = 0; i < nStreams; ++i) {
      size_t offset = i * streamBytes;
      size_t bitOffset = i * streamBitmapBytes;
      
      checkCuda(cuMemcpyHtoD(devBufferMem + offset, hostMem + offset, streamBytes));
      CUdeviceptr _mem = devBufferMem + offset;
      CUdeviceptr _tb = devBuffer_bitmap + bitOffset;
      void *KernelParams[] = { 
                                (void*)&_mem, 
                                (void*)&_tb, 
                                (void*)&devBufferRet,
      };  
      checkCuda(cuLaunchKernel(function, gridSizeX, gridSizeY, gridSizeZ,
                                  blockSizeX, blockSizeY, blockSizeZ, 0, 0,
                                  &KernelParams[0], NULL));
      checkCuda(cuMemcpyDtoH(host_bitmap + bitOffset, devBuffer_bitmap + bitOffset, streamBitmapBytes));    
    }       

  }
  checkCuda( cuEventRecord(stopEvent, 0) );
  checkCuda( cuEventSynchronize(stopEvent) );
  checkCuda( cuEventElapsedTime(&ms, startEvent, stopEvent) );
  printf("Time for sequential V2 transfer and execute (ms): %f ms; %fk exec / second\n", ms, n * ROUND / ms);

  // async mode
  checkCuda( cuEventRecord(startEvent, 0) );
  for (int _ = 0; _ < ROUND; ++_) {
    cuMemsetD8(devBufferMem, 'A', bytes);
    cuMemsetD8(devBuffer_bitmap, '\x00', bitmapBytes);
    for (size_t i = 0; i < nStreams; ++i) {
        size_t offset = i * streamBytes;
        size_t bitOffset = i * streamBitmapBytes;

        CUdeviceptr _mem = devBufferMem + offset;
        cuMemcpyHtoDAsync(devBufferMem+offset, hostMem+offset, streamBytes, streams[i]);

        CUdeviceptr _tb = devBuffer_bitmap + bitOffset;
        void *KernelParams[] = {(void*)&_mem, &_tb, &devBufferRet};  

        checkCuda(cuLaunchKernel(function, gridSizeX, gridSizeY, gridSizeZ,
                                  blockSizeX, blockSizeY, blockSizeZ, 0, streams[i],
                                  &KernelParams[0], NULL));
    }
  }
  checkCuda( cuEventRecord(stopEvent, 0) );
  checkCuda( cuEventSynchronize(stopEvent) );
  checkCuda( cuEventElapsedTime(&ms, startEvent, stopEvent) );
  printf("Time for asynchronous transfer and execute (ms): %f ms; %fk exec / second\n", ms, n * ROUND / ms);
  
  cuMemGetInfo(&memfree, &memtotal);
  std::cerr << "free mem = " << memfree << " B, total mem = " << memtotal << "B\n";

  // Retrieve device data
  uint8_t hostRes[hostReturndataSize]{0};
  checkCuda(
      cuMemcpyDtoH(&hostRes[0], devBufferRet, sizeof(char) * hostReturndataSize));
  printbytes(&hostRes[0], hostReturndataSize);

  // Clean-up
  free(streams);
  cuMemFreeHost(hostMem);
  cuMemFreeHost(host_bitmap);

  cuMemFree(devBufferMem);
  cuMemFree(devBufferRet);
  cuMemFree(devBuffer_bitmap);
  cuModuleUnload(cudaModule);
  cuCtxDestroy(context);

  return 0;
}