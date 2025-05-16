#ifndef _PTXBUILTINS_H
#define _PTXBUILTINS_H

// #define DEBUG_CUDA_FLAG

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef unsigned _ExtInt(1) bool;
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long uint64_t;
typedef signed long int64_t;
typedef unsigned _ExtInt(256) uint256_t;
typedef signed _ExtInt(256) int256_t;

typedef struct { uint256_t key; uint256_t val; } Slot_t;

#define true ((bool)1)
#define false ((bool)0)

#define likely(_x)   __builtin_expect(!!(_x), 1)
#define unlikely(_x)  __builtin_expect(!!(_x), 0)

/* Address Space	Memory Space */
#define AS0 __attribute__((address_space(0))) /* Generic  */
#define AS1 __attribute__((address_space(1))) /* Global   */
#define AS2 __attribute__((address_space(2))) /* Internal */
#define AS3 __attribute__((address_space(3))) /* Shared   */
#define AS4 __attribute__((address_space(4))) /* Constant */
#define AS5 __attribute__((address_space(5))) /* Local    */

/*
  threadId.*	@llvm.nvvm.read.ptx.sreg.tid.*
  blockIdx.*	@llvm.nvvm.read.ptx.sreg.ctaid.*
  blockDim.*	@llvm.nvvm.read.ptx.sreg.ntid.*
  gridDim.*	  @llvm.nvvm.read.ptx.sreg.nctaid.*
  ===> blockDim.x * blockIdx.x + threadId.x
*/
int32_t __nvvm_read_ptx_sreg_tid_x(); // threadId.x
int32_t __nvvm_read_ptx_sreg_ctaid_x(); // blockIdx.x
int32_t __nvvm_read_ptx_sreg_ntid_x(); // blockDim.x
#define ThreadId (__nvvm_read_ptx_sreg_tid_x() + (__nvvm_read_ptx_sreg_ctaid_x() * __nvvm_read_ptx_sreg_ntid_x()))

void __syncthreads();


printf(const char *, ...);

#endif /* _PTXBUILTINS_H */