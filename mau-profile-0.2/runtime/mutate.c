#include "mutate.h"
#include "storage.h"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/* Maximum size of input file, in bytes (keep under 100MB): */
#define MAX_FILE            (SEED_SIZE)

#define ARITH_MAX           35

/* Caps on block sizes for cloning and deletion operations. Each of these
   ranges has a 33% probability of getting picked, except for the first
   two cycles where smaller blocks are favored: */

#define HAVOC_BLK_SMALL     32
#define HAVOC_BLK_MEDIUM    128
#define HAVOC_BLK_LARGE     1500

/* Extra-large blocks, selected very rarely (<5% of the time): */

#define HAVOC_BLK_XL        32768

#define SWAP16(_x) ({ \
    u16 _ret = (_x); \
    (u16)((_ret << 8) | (_ret >> 8)); \
  })

#define SWAP32(_x) ({ \
    u32 _ret = (_x); \
    (u32)((_ret << 24) | (_ret >> 24) | \
          ((_ret << 8) & 0x00FF0000) | \
          ((_ret >> 8) & 0x0000FF00)); \
  })

#define SWAP64(_x) ({ \
    u64 _ret = (_x); \
    (u64)(((_ret << 56) & 0xff00000000000000ULL) | \
          ((_ret << 40) & 0x00ff000000000000ULL) | \
          ((_ret << 24) & 0x0000ff0000000000ULL) | \
          ((_ret << 8)  & 0x000000ff00000000ULL) | \
          ((_ret >> 8)  & 0x00000000ff000000ULL) | \
          ((_ret >> 24) & 0x0000000000ff0000ULL) | \
          ((_ret >> 40) & 0x000000000000ff00ULL) | \ 
          ((_ret >> 56) & 0x00000000000000ffULL)); \
})

/* List of interesting values to use in fuzzing. */
#define INTERESTING_8 \
  -128,          /* Overflow signed 8-bit when decremented  */ \
  -1,            /*                                         */ \
   0,            /*                                         */ \
   1,            /*                                         */ \
   16,           /* One-off with common buffer size         */ \
   32,           /* One-off with common buffer size         */ \
   64,           /* One-off with common buffer size         */ \
   100,          /* One-off with common buffer size         */ \
   127           /* Overflow signed 8-bit when incremented  */

#define INTERESTING_16 \
  -32768,        /* Overflow signed 16-bit when decremented */ \
  -129,          /* Overflow signed 8-bit                   */ \
   128,          /* Overflow signed 8-bit                   */ \
   255,          /* Overflow unsig 8-bit when incremented   */ \
   256,          /* Overflow unsig 8-bit                    */ \
   512,          /* One-off with common buffer size         */ \
   1000,         /* One-off with common buffer size         */ \
   1024,         /* One-off with common buffer size         */ \
   4096,         /* One-off with common buffer size         */ \
   32767         /* Overflow signed 16-bit when incremented */

#define INTERESTING_32 \
  -2147483648LL, /* Overflow signed 32-bit when decremented */ \
  -100663046,    /* Large negative number (endian-agnostic) */ \
  -32769,        /* Overflow signed 16-bit                  */ \
   32768,        /* Overflow signed 16-bit                  */ \
   65535,        /* Overflow unsig 16-bit when incremented  */ \
   65536,        /* Overflow unsig 16 bit                   */ \
   100663045,    /* Large positive number (endian-agnostic) */ \
   2147483647    /* Overflow signed 32-bit when incremented */
static int8_t AS4 interesting_8[]  = { INTERESTING_8 };
static int16_t AS4 interesting_16[] = { INTERESTING_8, INTERESTING_16 };
static int32_t AS4 interesting_32[] = { INTERESTING_8, INTERESTING_16, INTERESTING_32 };

enum AS4 ArgType {
    /// Empty
    TvNone,
    /// All 256-bit types (uint8, uint16, uint32, uint64, uint128, uint256)
    TvInt8, TvInt16, TvInt24, TvInt32, TvInt40, TvInt48, TvInt56, TvInt64, 
    TvInt72, TvInt80, TvInt88, TvInt96, TvInt104, TvInt112, TvInt120, TvInt128, 
    TvInt136, TvInt144, TvInt152, TvInt160, TvInt168, TvInt176, TvInt184, TvInt192, 
    TvInt200, TvInt208, TvInt216, TvInt224, TvInt232, TvInt240, TvInt248, TvInt256,
    /// Address
    TvAddress,
    /// Bool
    TvBool,
    /// string or bytes
    TvStrBytes1, TvStrBytes2, TvStrBytes3, TvStrBytes4, 
    TvStrBytes5, TvStrBytes6, TvStrBytes7, TvStrBytes8, 
    TvStrBytes9, TvStrBytes10, TvStrBytes11, TvStrBytes12,
    TvStrBytes13, TvStrBytes14, TvStrBytes15, TvStrBytes16, 
    TvStrBytes17, TvStrBytes18, TvStrBytes19, TvStrBytes20,
    TvStrBytes21, TvStrBytes22, TvStrBytes23, TvStrBytes24, 
    TvStrBytes25, TvStrBytes26, TvStrBytes27, TvStrBytes28,
    TvStrBytes29, TvStrBytes30, TvStrBytes31, TvStrBytes32,
    /// Offset for the dynamic variables
    TvOffset,
    /// Length for the array
    TvLength,
};


uint32_t cbconstants_length = 0;
uint8_t cbconstants[2048][32];
uint8_t cbconstant_sizes[2048]; // max to 32 bytes

volatile uint8_t AS1 callers_pool[64][32];
volatile uint32_t AS1 callers_pool_len = 0;
volatile uint8_t AS1 addresses_pool[64][32];
volatile uint32_t AS1 addresses_pool_len = 0;

// #define MAGICSIZE 9
// uint8_t AS4 MAGIC[MAGICSIZE] = { 0, 1, 0x3f, 0x40, 0x41, 0x7f, 0x80, 0x81, 0xff };

// address space 1
#define FLIP_BIT(_ar, _b) do { \
  uint8_t *_arf = (uint8_t*)(_ar); \
  uint32_t _bf = (_b); \
  _arf[(_bf) >> 3] ^= (128 >> ((_bf) & 7)); \
} while (0)


void cuhavoc(uint8_t *out_buf, uint32_t temp_len);

// uint8_t pickBoundaryByte() {
//   return MAGIC[UR(MAGICSIZE)];
// }

// void pickInterestingElemBytes(uint8_t *data, ArgType elemType, uitn32_t *rs) {
//   switch (elemType) {
//     case Int8 ... Int256:
//     case UInt8 ... UInt256:
//         uint8_t width = 1 + (elemType - Int8) % 32;
//         if (width == 1) *data = pickBoundaryByte(rs);
//         else pickBoundaryIntBytes(data, width, rs);
//         break;
//     case Address:
//         // memset(data, ACCOUNTPOOL[randInt(rs, len(ACCOUNTPOOL))], 32);
//         break;
//     case Bool:
//         memset(data, 0, 32);
//         break;
//     case Byte:
//         *data = pickBoundaryByte(rs);
//         break;
//     case String:
//         data[data[31]] = 0x41;
//         data[data[32]] = 0x42;
//         data[data[33]] = 0x43;
//         break;
//     case Array:
//         // Array type not allowed for an element
//         break;
//   }
// }


// void pickBoundaryIntBytes(uint8_t *data, uint32_t width) {
//   if (width < 2)  return;

//   const uint8_t heads[9] = {0x00, 0x00, 0x3F, 0x40, 0x40, 0x7F, 0x80, 0x80, 0xFF};
//   const uint8_t tails[9] = {0x00, 0x01, 0xFF, 0x00, 0x01, 0xFF, 0x00, 0x01, 0xFF};
//   const uint8_t mids[9]  = {0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF};
  
//   data[32 - width] = heads[UR(9)]; 
//   __builtin_memset(&data[32 - width + 1], mids[UR(9)], width - 2);
//   // FIXME: appending the tail byte will cause CUDA crash.
//   data[31] = tails[UR(9)]; 
// }


void mutateMem(const uint8_t *buff, const enum ArgType elemType) {
  uint32_t width;
  uint8_t *p;

  switch (elemType) {
    case TvNone: 
    case TvOffset:
    case TvLength: 
      // mutate nothing
      break;
    case TvInt8 ... TvInt256: 
      width = 1 + elemType - TvInt8;  
      if (width == 1) {
        p = buff + 31;
        *p = (uint8_t)UR(0x100); 
      }
      else cuhavoc(buff + 32 - width, width);
      break;
    case TvAddress: {
      if (UR(100) < 90) {
        // printf("[CUDA] addresses_pool_len = %d\n", addresses_pool_len);
        uint32_t idx = UR(100) < 80 ? 0 : UR(addresses_pool_len);
        __builtin_memcpy(buff + 12, addresses_pool[idx], 20);
      } else {
        __builtin_memset(buff, 0, 32);
      }
      break;
    }
    case TvBool: 
      p = buff + 31;
      *p = (uint8_t)UR(2);
      break;
    case TvStrBytes1 ... TvStrBytes32: 
      width = 1 + elemType - TvStrBytes1;
      // printf("[-] Kernel mutating width = %d\n", width); 
      // mutate in big endiance
      cuhavoc(buff, width);
      break;
    default:
  #ifdef DEBUG_CUDA_FLAG
      if (ThreadId == 0)
        printf("[Kernel] Invalid mutation type: %u\n", elemType);
  #endif
      break;
  }
}

bool is_mutable(const enum ArgType Type) {
  bool mut = false;
  switch (Type) {
    case TvNone: 
    case TvOffset:
    case TvLength:
      mut = false;
      break;
    case TvInt8 ... TvInt256: 
    case TvAddress: 
    case TvBool: 
    case TvStrBytes1 ... TvStrBytes32: 
      mut = true;
      break;
  }
  return mut;
}

void mutateCaller(uint8_t *caller) {
  uint32_t idx;
  if (UR(100) < 80) {
    idx = 0;
  } else {
    idx = UR(callers_pool_len);
  }
  __builtin_memcpy(caller, callers_pool[idx], 32);
}

void mutateCallvalue(uint8_t *callvalue) {
  uint32_t mutate_tries = 1 << (1 + UR(7));
  for (uint32_t _ = 0; _ < mutate_tries; _++) {
    cuhavoc(callvalue + 16, 16);
  }
}

void mutateCalldata(uint8_t *buff, const uint32_t argNum) {
  uint32_t i = UR(argNum);
  uint32_t tries = argNum;
  while (tries--) {
    const enum ArgType elemType = (enum ArgType) argTypeMap[i];
    if (is_mutable(elemType)) {
      // printf("[Kernel] argNum =%d; i = %d\targs types => %u\n", argNum, i, elemType);
      uint32_t mutate_tries = 1 << (1 + UR(7));
      for (uint32_t _ = 0; _ < mutate_tries; _++) {
        mutateMem(buff + 32 * i, elemType);
      }
      break;
    } else {
      i = (i + 1) % argNum;
    }
  }
}

// static u32 choose_block_len(u32 limit) {
//   u32 queue_cycle = 2; // added
//   u32 run_over10m = 12; // added
//   u32 min_value, max_value;
//   u32 rlim = MIN(queue_cycle, 3);

//   if (!run_over10m) rlim = 1;

//   switch (UR(rlim)) {

//     case 0:  min_value = 1;
//              max_value = HAVOC_BLK_SMALL;
//              break;

//     case 1:  min_value = HAVOC_BLK_SMALL;
//              max_value = HAVOC_BLK_MEDIUM;
//              break;

//     default: 
//              if (UR(10)) {
//                min_value = HAVOC_BLK_MEDIUM;
//                max_value = HAVOC_BLK_LARGE;
//              } else {
//                min_value = HAVOC_BLK_LARGE;
//                max_value = HAVOC_BLK_XL;
//              }
//   }
//   if (min_value >= limit) min_value = 1;
//   return min_value + UR(MIN(max_value, limit) - min_value + 1);
// }


void cuhavoc(uint8_t *out_buf, uint32_t temp_len) {
  switch (UR(18)) {
    case 0: {
      /* Flip a single bit somewhere. Spooky! */
      FLIP_BIT(out_buf, UR(temp_len << 3));
      break;
    }
    case 1: {
      /* Flip a single byte somewhere.  */
      out_buf[UR(temp_len)] ^= 0xff;
      break;
    }
    case 2: {
      /* Byte increment mutation for inputs with a bytes vector */
      out_buf[UR(temp_len)] += 1;
      break;
    }
    case 3: {
      /* Byte decresement mutation for inputs with a bytes vector */
      out_buf[UR(temp_len)] -= 1;
      break;
    }
    case 4: {
      /* Byte negate mutation for inputs with a bytes vector. */
      uint32_t idx =  UR(temp_len);
      out_buf[idx] = (~out_buf[idx]) + 1;
      break;
    }
    case 5: {
      /* Byte random mutation for inputs with a bytes vector */
      out_buf[UR(temp_len)] = UR(0x100) & 0xff;
      break;
    }
    case 6: {
      /* Randomly add/sub to byte. */
      if (UR(2)) {
        out_buf[UR(temp_len)] += 1 + UR(ARITH_MAX);
      } else {
        out_buf[UR(temp_len)] -= 1 + UR(ARITH_MAX);
      }
      break;
    }
    case 7: {
      /* Randomly add/sub to word, big endian. */
      if (temp_len < 2) break;
      u32 pos = UR(temp_len - 1);
      u16 num = 1 + UR(ARITH_MAX);
      uint16_t data = 0;
      __builtin_memcpy(&data, out_buf + pos, 2);
      if (UR(2)) {
        data = SWAP16(SWAP16(data) + num);
      } else {
        data = SWAP16(SWAP16(data) - num);
      }
      __builtin_memcpy(out_buf + pos, &data, 2);
      break;
    }
    case 8: {
      /* Randomly add/subtract from dword, big endian. */
      if (temp_len < 4) break;
      u32 pos = UR(temp_len - 3);
      u32 num = 1 + UR(ARITH_MAX);
      uint32_t data = 0;
      __builtin_memcpy(&data, out_buf + pos, 4);
      if (UR(2)) {
        data = SWAP32(SWAP32(data) + num);
      } else {
        data = SWAP32(SWAP32(data) - num);
      }
      __builtin_memcpy(out_buf + pos, &data, 4);
      break;
    }
    case 9: {
      /* Randomly add/subtract from qword, big endian. */
      if (temp_len < 8) break;
      u32 pos = UR(temp_len - 7);
      u64 num = 1 + UR(ARITH_MAX);
      uint64_t data = 0;
      __builtin_memcpy(&data, out_buf + pos, 8);
      if (UR(2)) {
        data = SWAP64(SWAP64(data) + num);
      } else {
        data = SWAP64(SWAP64(data) - num);
      }
      __builtin_memcpy(out_buf + pos, &data, 8);
      break;
    }
    case 10: { 
      /* Set byte to interesting value. */
      out_buf[UR(temp_len)] = interesting_8[UR(sizeof(interesting_8))];
      break;
    }
    case 11: {
      /* Set word to interesting value, randomly choosing endian. */
      if (temp_len < 2) break;
      uint16_t data = interesting_16[UR(sizeof(interesting_16) >> 1)];
      if (UR(2)) {
        __builtin_memcpy(out_buf + UR(temp_len - 1), &data, 2);
      } else {
        data = SWAP16(data);
        __builtin_memcpy(out_buf + UR(temp_len - 1), &data, 2);
      }
      break;
    }
    case 12: {
      /* Set dword to interesting value, randomly choosing endian. */
      if (temp_len < 4) break;
      uint32_t data = interesting_32[UR(sizeof(interesting_32) >> 2)];
      if (UR(2)) {
        __builtin_memcpy(out_buf + UR(temp_len - 3), &data, 4);
      } else {  
        data = SWAP32(data);
        __builtin_memcpy(out_buf + UR(temp_len - 3), &data, 4);
      }
      break;
    }
    case 13: {
      /* Bytes set mutation for inputs with a bytes vector */
      if (temp_len == 0) break;
      uint32_t off = UR(temp_len);
      uint64_t len = 1 + UR(MIN(16, temp_len - off));
      uint8_t val = out_buf[UR(temp_len)];
      __builtin_memset(out_buf + off, val, len);
      break;
    }
    case 14: {
      /* Bytes random set mutation for inputs with a bytes vector */
      if (temp_len == 0) break;
      uint32_t off = UR(temp_len);
      uint64_t len = 1 + UR(MIN(16, temp_len - off));
      uint8_t val = (uint8_t)(UR(0x100) & 0xff);
      __builtin_memset(out_buf + off, val, len);
      break;
    }
    case 15: {
      /* Bytes swap mutation for inputs with a bytes vector */
      if (temp_len <= 1) break;
      uint32_t first = UR(temp_len);
      uint32_t second = UR(temp_len);
      uint64_t len = 1 + UR(temp_len - MAX(first, second));
      
      uint8_t tmp[TRANSACTION_SIZE];
      // uint8_t tmp[1024];
      __builtin_memcpy(tmp, out_buf + first, len);
      __builtin_memcpy(out_buf + first, out_buf + second, len);
      __builtin_memcpy(out_buf + second, tmp, len);
      break;
    }
    case 16: {
      /* a mutator that mutates the input to a constant in the contract */

      // struct constant_vec_t constants[32];
      // constants[0].size = 32;
      // *(uint32_t*)constants[0].data = 0x12d312;
      // uint8_t constants[1024][32] = {{1, 234}, {2, 245}, {0, 248}, {1, 124}, {2, 165}, {78, 72, 123, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {2, 154}, {3, 197}, {0, 139}, {1, 185}, {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}, {3, 213}, {2, 191}, {2, 200}, {18, 211, 18}, {2, 15}, {1, 239}, {2, 137}, {243, 36}, {3, 46}, {3, 156}, {4, 65}, {3, 235}, {105, 112, 102, 115, 88}, {2, 237}, {0, 32}, {0, 173}, {3, 145}, {255, 255, 129}, {2, 175}, {2, 58}, {2, 78}, {3, 138}, {3, 190}, {0, 87}, {174, 111, 6, 248}, {1, 244}, {0, 123}, {3, 93}, {3, 167}, {1, 107}, {3, 36}, {2, 95}, {0, 74}, {1, 35}, {19, 51, 55}, {2, 67}, {1, 51}, {188, 53, 243, 0, 130, 75, 82, 132, 174, 209, 235, 98, 158, 179, 228, 97, 82, 122, 65, 153, 134, 220, 253, 81, 96, 179, 217, 239, 139, 66, 100, 115}, {0, 96}, {2, 22}, {0, 69}, {3, 104}, {1, 0, 0, 0, 0, 0, 0, 0, 240, 0}, {2, 98}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {3, 115}, {0, 189}, {3, 224}, {3, 200}, {8, 195, 121, 160, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {2, 43}, {4, 4}, {2, 211}, {0, 232}, {72, 101, 108, 108, 111, 32, 67, 111, 110, 116, 114, 97, 99, 116, 115, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
      // uint8_t sizes[1024] = {3, 2, 2, 2, 2, 2, 32, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 32, 2, 4, 10, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 32, 2, 2, 2, 10, 32, 2, 2, 2, 2, 20, 2, 2, 3, 2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, };
      
      // uint8_t constants[1024][32] = {{0x10}, {0x13}};
      // uint8_t sizes[1024] = {1, 1};  
      if (cbconstants_length > 0) {
        uint32_t idx = UR(cbconstants_length);
        uint8_t *constant = &cbconstants[idx][0];
        uint8_t constant_len = cbconstant_sizes[idx];
        if (temp_len <= constant_len) {
          __builtin_memcpy(out_buf, constant, temp_len);
        } else {
          __builtin_memset(out_buf, 0, temp_len - constant_len);
          __builtin_memcpy(out_buf + temp_len - constant_len, constant, constant_len);
        }
      }
      // printf("contsant mutation out_buff#%d: %x%x%x\n", temp_len, out_buf[29], out_buf[30], out_buf[31]);
      // printbytes(out_buf, temp_len);
      break;
    }
    case 17: {
      uint32_t l2i = __snap_map[ThreadId];
      if (l2snap_lens[l2i] > 0) {
        Slot_t AS1 *l2snapshot = l2snaps[l2i];
        uint32_t idx = UR(l2snap_lens[l2i]);
        if (UR(100) < 90) {
          uint8_t *val = (uint8_t*)&l2snapshot[idx].val;
          __builtin_memcpy(out_buf, val + 32 - temp_len, temp_len);
        } else {
          uint8_t *key = (uint8_t*)&l2snapshot[idx].key;
          __builtin_memcpy(out_buf, key + 32 - temp_len, temp_len);
        }
      }
      break;
    }
  }
}