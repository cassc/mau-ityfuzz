#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "ptx_builtins.h"
#include "../config.h"

extern uint32_t AS1 __snap_map[NJOBS];

// runtime snapshot
extern Slot_t AS1 l1snaps[NJOBS][ONE_SNAP_LEN];
extern uint8_t AS1 l1snap_lens[NJOBS];

// root snapshot
extern Slot_t AS1 l2snaps[NJOBS*32][ONE_SNAP_LEN];
extern uint8_t AS1 l2snap_lens[NJOBS*32];

void __device_sstore(uint256_t *pkey, uint256_t *pval);
void __device_sload(uint256_t *pkey, uint256_t *pval);

#endif  /* _STORAGE_H_ */