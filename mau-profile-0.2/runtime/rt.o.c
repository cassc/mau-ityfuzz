#include "../config.h"

#include "ptx_builtins.h"

#include "keccak256.h"
#include "random.h"
#include "mutate.h"
#include "storage.h"

void bytesToString(char buf[], uint8_t const bytes[], uint32_t const _size) {
  char hexmap[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  // char *buf = (char*)malloc(_size * 2 + 1);

  for (uint32_t i = 0; i < _size; ++i) {
    uint8_t byte = bytes[i];
    buf[2*i + 1]  = hexmap[(byte & 0xf)];
    byte >>= 4;
    buf[2*i]      = hexmap[byte & 0xf];
  }
  buf[_size*2] = '\x00';
  // printf("%s. hex bytes: %s\n", msg, buf);
  // free(buf);
}

void printbytes(char msg[], uint8_t const bytes[], uint32_t const _size) {
  char hexmap[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  
  char *buf = (char*)malloc(_size * 2 + 1);

  for (uint32_t i = 0; i < _size; ++i) {
    uint8_t byte = bytes[i];
    buf[2*i + 1]  = hexmap[byte & 0xf];
    byte >>= 4;
    buf[2*i]      = hexmap[byte & 0xf];
  }
  buf[_size*2] = '\x00';
  printf("%s. hex bytes: %s\n", msg, buf);
  free(buf);
}


#ifdef __cplusplus

extern "C" {
#endif  /* __cplusplus */

uint64_t AS4 cuTimestamp = 0;

// n_wrap
// uint32_t AS4 cuNwrap = 1;



uint8_t AS1 *__bitmaps[NJOBS];
uint8_t AS1 __virgin_bits[MAP_SIZE];

uint64_t AS1 __signals = 0;
uint8_t AS1 __hnbs[NJOBS] = {};

uint8_t AS1 __cov_bits[MAP_SIZE];


void __clear_bitmap(int8_t AS1 *src) {
  uint64_t AS1 *p = (uint64_t AS1 *)src;
  uint32_t i = (MAP_SIZE >> 3);
  while (i--) {
    *(uint64_t*)p = 0;
    ++p;
  }
  return;
}

void __device_sha3(uint8_t *data, uint32_t length, uint8_t *presult) {
  keccak256(data, length, presult);
  return;
}

void __power_word(uint256_t *bPtr, uint256_t *ePtr, uint256_t *rPtr) {
  uint256_t R = 1;
  uint256_t B = *(uint256_t*)bPtr;
  uint256_t E = *(uint256_t*)ePtr;
  while (E != 0) {
    if ((E & 1) == 1)
      R *= B;
    B *= B;
    E /= 2;
  }
  *(uint256_t*)rPtr = R;
  return;
}

void __device_calldatacpy(uint8_t *dst, uint64_t dstOffset, uint8_t *calldata, 
                          const uint64_t dataOffset, const uint64_t size) {
  uint64_t DataMaxLength = TRANSACTION_SIZE - 12;
  if (!size || dstOffset + size < dstOffset || dstOffset + size >= EVM_MEM_SIZE
      || dataOffset + size < dataOffset || dataOffset + size >= DataMaxLength) 
    return;
  uint64_t start = MIN(dataOffset, DataMaxLength);
  uint64_t end = MIN(start + size, DataMaxLength);  
  for (uint64_t i = 0; i < end - start; i++) 
    *(uint8_t*)(dst + dstOffset + i) = *(uint8_t*)(calldata + start + i);
}

void __device_calldataload(uint8_t *dst, uint8_t *calldata, 
                           const uint64_t dataOffset) {
  uint64_t DataMaxLength = TRANSACTION_SIZE - 12;
  if (dataOffset + 32 < dataOffset || dataOffset + 32 >= DataMaxLength) {
    *(uint256_t*)dst = 0;
    return;
  }
  for (uint8_t i = 0; i < 32; i++) 
    *(dst + i) = *(calldata + dataOffset + 31 - i);
}

void __device_mstore(uint8_t *mem, const uint64_t offset, 
                     uint8_t *valPtr8, const uint64_t size) {
  if (!size || (offset + size) < offset 
      || (offset >= EVM_MEM_SIZE - size))
    return;

  for (uint64_t i = 0; i < size; i++)
    *(uint8_t*)(mem + offset + i) = *(uint8_t*)(valPtr8 + size - 1 - i);
}

void __device_mload(uint8_t *mem, const uint64_t offset, uint256_t *valPtr) {
  if (offset >= (EVM_MEM_SIZE - 32)) {
    *(uint256_t*)valPtr = 0;
    return;
  }
  else {
    uint8_t *valPtr8 = (uint8_t*)valPtr;
    for (uint8_t i = 0; i < 32; i++)
      *(uint8_t*)(valPtr8 + i) = *(uint8_t*)(mem + offset + 31 - i);
    return;
  }
}


void __device_sstore(uint256_t *pkey, uint256_t *pval) { 
  // uint8_t bufkey[32*2+1];
  // bytesToString(bufkey, (uint8_t*)pkey, 32);
  // uint8_t bufval[32*2+1];
  // bytesToString(bufval, (uint8_t*)pval, 32);
  // printf("[Kernel#%d] sstore(%s, %s)\n", ThreadId, bufkey, bufval);

  uint32_t const tid = ThreadId;  
  Slot_t AS1 *snapshot = l1snaps[tid];
  uint8_t i = 0;
  for (i = 0; i < l1snap_lens[tid]; ++i) {
    if (snapshot[i].key == *pkey) {
      snapshot[i].val = *pval; 
      return;
    }
  }
  // find nothing, then insert a new slot
  snapshot[i].key = *pkey;
  snapshot[i].val = *pval;
  l1snap_lens[tid] += 1;
}

void __device_sload(uint256_t *pkey, uint256_t *pval) { 
  // uint8_t bufkey[32*2+1];
  // bytesToString(bufkey, (uint8_t*)pkey, 32);
  // uint8_t bufval[32*2+1];
  // bytesToString(bufval, (uint8_t*)pval, 32);
  // printf("[Kernel#%d] sload(%s, %s)\n", ThreadId, bufkey, bufval);

  uint32_t const tid = ThreadId;  
  Slot_t AS1 *snapshot = l1snaps[tid];

  uint256_t const key = *(uint256_t*)pkey;
  uint32_t i = 0;
  for (i = 0; i < l1snap_lens[tid]; ++i) {
    if (snapshot[i].key == key) {
      *pval = snapshot[i].val;
      return;
    }
  }

  // if find nothing again, search in L2 STORAGE again
  // uint32_t l2i = tid / (NJOBS / cuNwrap);
  uint32_t l2i = __snap_map[tid];
  // printf("CUDA => l2i = %d\n", l2i);
  Slot_t AS1 *l2snapshot = l2snaps[l2i];
  for (i = 0; i < l2snap_lens[l2i]; ++i) {
    if (l2snapshot[i].key == key) {
      *pval = l2snapshot[i].val;
      // printbytes("[Kernel] SLOAD:", (uint8_t*)(pval), 32);
      return;
    }
  }
  // at last, return default value 
  *pval = (uint256_t)0; // default value
  return;
}


uint64_t __simple_hash(uint64_t const _x) {
    uint64_t x = _x;
    x = (x ^ (x >> 30)) * (uint64_t)0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * (uint64_t)0x94d049bb133111eb;
    x = x ^ (x >> 31);
    return x;
}

uint32_t __hashint(uint32_t const old, uint32_t const val){
  uint64_t input = (((uint64_t)(old))<<32) | ((uint64_t)(val));
  return (uint32_t)(__simple_hash(input));
}

uint32_t __hashword(uint256_t const *val){
  uint64_t *ptr = (uint64_t*)val;
  uint64_t v0 = *(ptr++);
  uint64_t v1 = *(ptr++);
  uint64_t v2 = *(ptr++);
  uint64_t v3 = *ptr;
  uint64_t r = __simple_hash(v0) ^ __simple_hash(v1) ^ 
               __simple_hash(v2) ^ __simple_hash(v3);
  uint32_t hr = (uint32_t)(r >> 32);
  uint32_t lr = (uint32_t)(r & 0xffffffff);
  // printf("%d %d\n", hr, lr);
  return hr ^ lr;
}

static const uint8_t AS3 count_class_lookup8[256] = {
  [0]           = 0,
  [1]           = 1,
  [2]           = 2,
  [3]           = 4,
  [4 ... 7]     = 8,
  [8 ... 15]    = 16,
  [16 ... 31]   = 32,
  [32 ... 127]  = 64,
  [128 ... 255] = 128
};

void __classify_counts(uint64_t AS1 *mem) {
  uint32_t i = MAP_SIZE >> 3;

  while (i--) {
    /* Optimize for sparse bitmaps. */
    if (unlikely(*mem)) {
      uint8_t* mem8 = (uint8_t*)mem;
  
      mem8[0] = count_class_lookup8[mem8[0]];
      mem8[1] = count_class_lookup8[mem8[1]];
      mem8[2] = count_class_lookup8[mem8[2]];
      mem8[3] = count_class_lookup8[mem8[3]];
      mem8[4] = count_class_lookup8[mem8[4]];
      mem8[5] = count_class_lookup8[mem8[5]];
      mem8[6] = count_class_lookup8[mem8[6]];
      mem8[7] = count_class_lookup8[mem8[7]];
    }
    mem++;
  }
}

#define FF(_b)  (0xff << ((_b) << 3))
/* Count the number of bytes set in the bitmap. Called fairly sporadically,
   mostly to update the status screen or calibrate and examine confirmed
   new paths. */
static uint32_t count_bytes() {
  uint32_t AS1* ptr = (uint32_t AS1*)__cov_bits;
  uint32_t  i   = (MAP_SIZE >> 2);
  uint32_t  ret = 0;
  while (i--) {
    uint32_t v = *(ptr++);
    if (!v) continue;
    if (v & FF(0)) ret++;
    if (v & FF(1)) ret++;
    if (v & FF(2)) ret++;
    if (v & FF(3)) ret++;
  }
  return ret;
}

void updateBits(uint64_t AS1* flag) {
  uint32_t const tid = ThreadId;
  // printf("updateBits thread = %d\n", tid);
  uint64_t AS1 *current;
  uint64_t AS1 *virgin = ((uint64_t AS1 *)__virgin_bits) + tid;
  /* If the all virgin bits are touched (Bit 0), 
     we do not have to update it again. */
  for (uint32_t i = 0; i < NJOBS; i++) {
    current = ((uint64_t AS1 *)__bitmaps[i]) + tid; 
    // printf("#tid[%d] p[i=%d] %p-> current %p\n", tid, i, ((uint64_t AS1 *)__bitmaps[i]) , current);
    if (unlikely(*current) && unlikely(*current & *virgin)) {
      // new edges explored
      if (__hnbs[i] != EXECBUGGY) {
        // printf("[Kernel] interesting at thread %d\n", i);
        __hnbs[i] = EXECINTERESTING; 
      }
      *virgin &= ~*current;
    }
    *current = 0; // reset bitmap
    __syncthreads();
    // if (tid == 0 && __signals) {
    //   hnbs[__signals & 0xffffffff] = 1;
    // }
  }
  __syncthreads();

  // collect bugs
  // if (__signals) {
    *flag = __signals;
    // hnbs[(uint32_t)(__signals & 0xffffffff)] = 2;
    #ifdef DEBUG_CUDA_FLAG
      if (ThreadId == 0) printf("Kernel. fn updateBits(): SIGNAL => %llx\n", __signals);
    #endif
  // }
  return;
}

void addBugSet(uint64_t field) {
uint32_t tid = ThreadId;
#ifdef MAZE_ENABLE
  // for the daedaluzz oracle
  __hnbs[tid] = EXECBUGGY; 
  uint64_t BUG = 0xdeaddead;
  __signals = ((BUG << 32) | (uint32_t)tid); // a flag for fast fuzzing
  printf("log@%lld \"%lx000000000000000000000000000000000000000000000000\"\n", cuTimestamp, field);

#else 
  if (field == 0x133337)  {
    // for the test bug oracle
    __hnbs[tid] = EXECBUGGY; 
    uint64_t BUG = 0xdeaddead;
    __signals = ((BUG << 32) | (uint32_t)tid); // a flag for fast fuzzing
    printf("bug; target hit at thread#%d. __hnbs[tid] = %d\n", ThreadId, EXECBUGGY);
  }
#endif

//   uint32_t tid = ThreadId;
//   __hnbs[tid] = EXECBUGGY;
//   uint64_t BUG = 0xdeaddead;
// //     __signals = ((BUG << 32) | (uint32_t)tid); // a flag for fast fuzzing
//   printf("log@%lld \"%lx000000000000000000000000000000000000000000000000\"\n", cuTimestamp, field);

//   if (field == 0x133337)  {
//     // for the test bug oracle
//     printf("bug; target hit at thread#%d. __hnbs[tid] = %d\n", ThreadId, EXECBUGGY);
//   }

#ifdef DEBUG_CUDA_FLAG
    printf("[-] Kernel. thread %d hitted bug with %llx\n", tid, __signals);
#endif
}

/* there are `NJOBS` seeds in the `seedVec`. Each seed has 
  `seedWidth` transactions. Each transaction contain one calldata
*/
void parallel_mutate(uint8_t AS1 *dataVec, uint32_t _) {
  uint32_t tid = ThreadId;
  uint8_t AS1 *tx = dataVec + SEED_SIZE * tid;
  uint8_t *caller = (uint8_t*)tx;
  uint8_t *callvalue = (uint8_t*)(tx + 32);
  uint32_t const calldatasize = *(uint32_t*)(tx + 64);
  uint8_t *calldata = (uint8_t*)(tx + 68);

  if (calldatasize <= 4 || UR(100) > 90) {
    switch (UR(2)) {
    case 0:
      // printbytes("mutateCaller =>", caller, 32);
      //  if (calldatasize < 40000) break;
      mutateCaller(caller);
      break;
    case 1:
      // printbytes("mutateValue =>", callvalue, 32);
      mutateCallvalue(callvalue);
      // printbytes("mutateValue =>", callvalue, 32);
      break;
    }
  } else {
    // printf("mutating calldata\n");
    uint8_t *argument = calldata + 4;
    uint32_t argNum = (calldatasize - 4) / 32;

    // printbytes("[Kernel] Mutating", (uint8_t*)(calldata), calldatasize);
    mutateCalldata((uint8_t*)argument, argNum);
    // printbytes("[Kernel] Mutated", (uint8_t*)(calldata), calldatasize);
#ifdef DEBUG_CUDA_FLAG
    if (tid == 0) {
      printf("[-] Kernel mutated: tx[%d]calldata#%dB => ", tid, calldatasize); 
      printbytes("[Kernel] Mutated", (uint8_t*)(calldata), calldatasize);
    }
#endif
  }
  __syncthreads();
}

/// GPU function entry

/* execute the deployer bytecode in just-in-time */
// uint32_t deployer(Slot_t AS1 *_a, uint8_t AS1 *_b, 
//               uint64_t callvalue, uint32_t calldatasize, uint8_t *calldata,
//               uint8_t AS1 *bitmap, uint8_t AS1 *signal, bool mutex);

/* execute the runtime bytecode in just-in-time */
uint32_t contract(uint256_t* pcaller, uint256_t* pcallvalue, uint8_t *calldata, uint32_t calldatasize, uint8_t AS1 *bitmap);


/* the kernel function to deploy the smart contract.
   `tx` is the transaction. `opt` is an optional parameter 
    which has not been used. */
// void main_deployer(uint8_t AS1 *tx, uint8_t AS1 *opt) {
//   // if (ThreadId == 0) {
//   //   l2snap_lens[0] = 0;
//   //   uint64_t const callvalue = *(uint64_t*)tx;
//   //   uint32_t const calldatasize = *(uint32_t*)(tx + 8);
//   //   uint8_t *calldata = (uint8_t*)(tx + 12);
//   //   deployer(&l2snaps[0][0], &l2snap_lens[0], 
//   //       callvalue, calldatasize, calldata, 
//   //       __bitmaps[0], __bitmaps[0], false);
//   //   l3snap_len = l2snap_lens[0];
//   //   __builtin_memcpy(&l3snap[0], &l2snaps[0][0], l2snap_lens[0] * sizeof(Slot_t));
//   //   l2snap_lens[0] = 0;
//   // }
// }

/* the kernel function to evaluate several seeds in `seedVec` together.
   Each seed contain `seedWidth` transactions. */
void main_contract(uint8_t AS1 *dataVec, uint32_t _d) {

  uint32_t tid = ThreadId;
  __hnbs[tid] = EXECNONE;

  uint8_t AS1 *tx = dataVec + SEED_SIZE * tid;

  uint256_t caller = *(uint256_t*)tx;
  uint256_t callvalue = *(uint256_t*)(tx + 32);
  uint32_t const calldatasize = *(uint32_t*)(tx + 64);
  uint8_t *calldata = (uint8_t*)(tx + 68);

  // printf("[-] Kernel[%d]===> \tcalldatasize: %d \n\tcalldata:", 
  //   ThreadId, calldatasize); 
  // printbytes("[Kernel] Executed", (uint8_t*)calldata, calldatasize);

#ifdef DEBUG_CUDA_FLAG
    if (ThreadId == 0) {
      printf("[-] Kernel[%d]===> \tcalldatasize: %d \n\tcalldata:", 
        ThreadId, calldatasize); 
      printbytes("[Kernel] Executed", (uint8_t*)calldata, calldatasize);
      __syncthreads();
    }
#endif

    uint64_t const _ = 0; // TODO
    l1snap_lens[tid] = 0;
    uint32_t successed = contract(&caller, &callvalue, calldata, calldatasize, __bitmaps[tid]);
    // if (successed == 0) __hnbs[tid] = EXECREVERTED; // exec_reverted

    /* when all threads finished evaluating the transactions 
       with the same index, we move to next transaction index.
       In round `i`, thread `tid` are evaluating `seedVec[tid][i]`.
    */
    // __syncthreads();
}

#ifdef __cplusplus
}
#endif  /* __cplusplus */