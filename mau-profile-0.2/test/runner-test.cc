#include <assert.h>
#include <string.h>

#include <cstdio>
#include <fstream>
#include <iostream>

#include <json/json.h>
#include <cuda.h>

#include "../config.h"
#include "../runner/runner.h"
#include "debug.h"

enum ArgType {
  // int8 ... int256
  Int8 = 0, Int16, Int24, Int32, Int40, Int48, Int56, Int64, 
  Int72, Int80, Int88, Int96, Int104, Int112, Int120, Int128, 
  Int136, Int144, Int152, Int160, Int168, Int176, Int184, Int192, 
  Int200, Int208, Int216, Int224, Int232, Int240, Int248, Int256,
  // uint8 ... uint256
  UInt8 = 32, UInt16, UInt24, UInt32, UInt40, UInt48, UInt56, UInt64, 
  UInt72, UInt80, UInt88, UInt96, UInt104, UInt112, UInt120, UInt128, 
  UInt136, UInt144, UInt152, UInt160, UInt168, UInt176, UInt184, UInt192,
  UInt200, UInt208, UInt216, UInt224, UInt232, UInt240, UInt248, UInt256,
  // others
  Address = 64,
  Bool,
  Byte,
  String,
  Array8, Array16, Array24, Array32, Array40, Array48, Array56, Array64, 
  Array72, Array80, Array88, Array96, Array104, Array112, Array120, Array128, 
  Array136, Array144, Array152, Array160, Array168, Array176, Array184, Array192, 
  Array200, Array208, Array216, Array224, Array232, Array240, Array248, Array256,
  Offset,
};

struct slot_t {
  uint64_t key[4];
  uint64_t val[4];
};


typedef struct {
  uint8_t From[20];
  uint8_t To[20];
  uint8_t Value[32];
  uint8_t Data[TRANSACTION_SIZE+1];
  uint32_t Calldatasize;
  uint8_t Timestamp[32];
  uint8_t Blocknum[32];
} Transaction;

typedef struct {
  Transaction DeployTx;
  Transaction Txs[TRANSACTIONS_MAX_LENGTH];
  uint32_t Width;
} SeedData;

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

// void printbytes(uint8_t bytes[], uint32_t _size) {
//   for (uint32_t i = 0; i < _size; ++i) {
//     printf("%02hhx", bytes[i]);
//   }
//   printf("\n");
// }

#define checkCuda(err) do { \
  if (err != CUDA_SUCCESS) { \
  const char *info; \
  cuGetErrorString(err, &info); \
  const char *errName;  \
  cuGetErrorName(err, &errName);  \
  std::cerr << "[ERROR-" << err << "] " << errName << ": " << info << std::endl; \
  printf("\n    Stop location : %s(), %s:%u\n\n", __FUNCTION__, __FILE__, __LINE__); \
  abort(); \
  }\
} while (0)


bool loadSeedFromJson(std::string &path, SeedData &seed) {
  Json::Value root;
  Json::Reader JsonReader;
  std::ifstream Infile(path, std::ios::in);
  if (!Infile) FATAL("Fail to read: %s\n", path.c_str());
  bool Parsed = JsonReader.parse(Infile, root);
  if (!Parsed)  FATAL("Failed to resolve the Seed.");

  Json::Value DeployTx = root["DeployTx"];
  
  bytesfromhex(seed.DeployTx.From, DeployTx["From"].asString());
  bytesfromhex(seed.DeployTx.To, DeployTx["To"].asString());
  // bytesfromhex(seed.DeployTx.Value, DeployTx["Value"].asString());
  bytesfromhex(seed.DeployTx.Data, DeployTx["Data"].asString());
  seed.DeployTx.Calldatasize = DeployTx["Data"].asString().size() / 2;
  // bytesfromhex(seed.DeployTx.Timestamp, DeployTx["Timestamp"].asString());
  // bytesfromhex(seed.DeployTx.Blocknum, DeployTx["Blocknum"].asString());
  
  Json::Value Txs = root["Txs"];
  seed.Width = Txs.size();
  for (uint32_t i = 0; i < Txs.size(); i++) {
    bytesfromhex(seed.Txs[i].From, Txs[i]["From"].asString());
    bytesfromhex(seed.Txs[i].To, Txs[i]["To"].asString());
    // bytesfromhex(seed.Txs[i].Value, Txs[i]["Value"].asString());
 
    bytesfromhex(seed.Txs[i].Data, Txs[i]["Data"].asString());
    seed.Txs[i].Calldatasize = Txs[i]["Data"].asString().size() / 2;

    // bytesfromhex(seed.Txs[i].Timestamp, Txs[i]["Timestamp"].asString());
    // bytesfromhex(seed.Txs[i].Blocknum, Txs[i]["Blocknum"].asString());  
  }
  Infile.close();
  return true;
}

/// main - Program entry point
int main(int argc, char **argv) {
  int cuda_device = 0; // default to use GPU#0
  // cli parameters
  if (argc < 3) {
    printf("Usage: runner-test <path_to_kernel> <path_to_input> [gpu]\n");
    exit(-1);
  }
  std::string pathToKernel = argv[1];
  std::string pathToSeed = argv[2];
  if (argc == 4) cuda_device = atoi(argv[3]);

  // CUDA initialization

  
  InitCudaCtx(cuda_device, pathToKernel.c_str());
  std::cerr << "[+] Activated GPU# " << cuda_device << "\n";

  cuMallocAll();

  // prepare a seed for the evalution
  SeedData SeedRaw;
  loadSeedFromJson(pathToSeed, SeedRaw);
  uint32_t seedWidth = SeedRaw.Width;
  
  uint64_t ExecTotal = 0;
  float elapsedMs;
  CUevent startEvent, stopEvent;
  checkCuda(cuEventCreate(&startEvent, 0));
  checkCuda(cuEventCreate(&stopEvent, 0));
  checkCuda(cuEventRecord(startEvent, 0));

  // deploy transaction  
  Transaction deployTx = SeedRaw.DeployTx;
  setEVMEnv(deployTx.From, deployTx.To, deployTx.Value, deployTx.Timestamp, deployTx.Blocknum);
  
  uint64_t Callvalue = 0;

  cuDeployTx(Callvalue, deployTx.Data, deployTx.Calldatasize);
  cuDumpStorage(0);

  // serialize the orignal seed
  uint8_t rawSeed[SEED_SIZE];
  memset(rawSeed, 0, SEED_SIZE);
  for (uint32_t txi = 0; txi < seedWidth; txi++) {
    Transaction tx = SeedRaw.Txs[txi];
    uint32_t calldatasize = tx.Calldatasize;
    uint8_t *calldata = tx.Data;

    memcpy(rawSeed, tx.Data, calldatasize);
    cuExtSeeds(rawSeed, calldatasize);

    uint8_t argTypes[16] = {Array8};
    // evaluate the seeds
    uint32_t sigma = 1;
    uint64_t cov = 0;
    while (sigma--) {
      // cuMutate(seedWidth, argTypes, 1);
      ExecTotal += cuEvalTxn(calldatasize);
      cuDumpStorage(0);

      // uint64_t newCov = postGainCov();
      // if (newCov) {
      //   cov = newCov;
      //   printf("Coverage = %lu Edges\n", cov);
      //   for (uint32_t i = 0; i < NJOBS; i++) {
      //     memset(rawSeed, 0, SEED_SIZE);
      //     if (gainCov(i, rawSeed)) {
      //       // cuExtSeeds(rawSeed);
      //       printf("Intersting seed at #%d:", i);
      //       printbytes(rawSeed, calldatasize);
      //     }
      //   }
      // }
    }
  }

  checkCuda(cuEventRecord(stopEvent, 0));
  checkCuda(cuEventSynchronize(stopEvent));
  checkCuda(cuEventElapsedTime(&elapsedMs, startEvent, stopEvent));
  printf("Execution info: %f ms; %f k exec / second\n", 
    elapsedMs, elapsedMs > 0 ? ExecTotal / elapsedMs : 0);

  // Clean-up
  // DestroyCuda();

  return 0;
}