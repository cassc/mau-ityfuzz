#ifndef _MUTATE_H_
#define _MUTATE_H_

#include "ptx_builtins.h"
#include "../config.h"
#include "random.h"

uint8_t AS1 argTypeMap[64];
typedef unsigned char uint8_t;
// extern uint32_t AS1 __snap_map[NJOBS];
// // root snapshot
// extern AS1 l2snaps[NJOBS*32][ONE_SNAP_LEN];
// extern uint8_t AS1 l2snap_lens[NJOBS*32];

void mutateCalldata(uint8_t *calldata, const uint32_t argNum);
void mutateCallvalue(uint8_t *callvalue);
void mutateCaller(uint8_t *caller);

#endif  /* _MUTATE_H_ */