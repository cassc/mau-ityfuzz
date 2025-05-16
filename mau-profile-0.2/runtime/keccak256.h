#ifndef __KECCAK256_H_
#define __KECCAK256_H_

/* #include <string.h> */
#define sha3_max_permutation_size 25
#define sha3_max_rate_in_qwords 24

// type def
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
typedef uint64_t size_t;

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

void keccak256(uint8_t *_input, uint32_t _size, uint8_t *_output);


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __KECCAK256_H_ */
