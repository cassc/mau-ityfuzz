#ifndef _HAVE_TYPES_H
#define _HAVE_TYPES_H

#include <vector>

struct WordDef {
  // bool sign = false;
  bool pad_l = false;
  int8_t len = 0; // bytes
  // uint32_t mask = 0;  /* a 32-bit mask indicating which bytes in a word are mutable. */
};

struct slot_t {
  uint64_t key[4];
  uint64_t val[4];
};

struct queue_entry_t {
  uint8_t *seed = nullptr;                 /* Input content                    */ 
  size_t len;                            /* Input length                     */
  uint64_t callvalue;                   /* callvalue */
  std::vector<uint8_t> mask;      

  bool    was_fuzzed,                      /* Had any fuzzing done yet?        */
          has_new_cov,                     /* Triggers new coverage?           */
          passed_det;

  uint32_t bitmap_size,                    /* Number of bits set in bitmap     */
           exec_cksum;                     /* Checksum of the execution trace  */

  uint64_t exec_us,                        /* Execution time (us)              */
           handicap,                       /* Number of queue cycles behind    */
           depth;                          /* Path depth                       */

  slot_t *storage;
  uint32_t storage_size;

  struct queue_entry_t *next = nullptr;    /* Next element, if any             */
};

struct chain_t {
  uint8_t *seed = nullptr;                          /* Input content */ 
  struct chain_t *next = nullptr;         /* Next element, if any             */
};


#endif /* ! _HAVE_TYPES_H */