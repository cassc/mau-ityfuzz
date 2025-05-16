#ifndef _RANDOM_H_
#define _RANDOM_H_

#include "../config.h"
#include "ptx_builtins.h"

int32_t AS1 cuda_states[NJOBS]; // global address

int32_t rand() { 
  uint32_t tid = ThreadId;
  unsigned int next = cuda_states[tid];
  int result;

  next *= 1103515245;
  next += 12345;
  result = (unsigned int) (next / 65536) % 2048;

  next *= 1103515245;
  next += 12345;
  result <<= 10;
  result ^= (unsigned int) (next / 65536) % 1024;

  next *= 1103515245;
  next += 12345;
  result <<= 10;
  result ^= (unsigned int) (next / 65536) % 1024;
   
  cuda_states[tid] = next;
  return result;
}

static inline uint32_t UR(uint32_t limit) { return rand() % limit; }

#endif  /* _RANDOM_H_ */