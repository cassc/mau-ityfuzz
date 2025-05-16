#include "storage.h"

uint32_t AS1 __snap_map[NJOBS];

// runtime snapshot
Slot_t AS1 l1snaps[NJOBS][ONE_SNAP_LEN] = {};
uint8_t AS1 l1snap_lens[NJOBS] = {};

// root snapshot
Slot_t AS1 l2snaps[NJOBS*32][ONE_SNAP_LEN] = {};
uint8_t AS1 l2snap_lens[NJOBS*32] = {};

