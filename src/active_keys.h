#ifndef __ACTIVE_KEYS_H__
#define __ACTIVE_KEYS_H__

#include "bound_key.h"

#define NUM_MODIFIERS 8
#define MAX_KEYS      6
#define MAX_ACTIVE_CELLS (MAX_KEYS + NUM_MODIFIERS)

typedef struct
{
  BoundKey  keys[MAX_ACTIVE_CELLS];
  uint8_t   num_keys;
  uint8_t   curr_key;
} ActiveKeys;

void      ActiveKeys__reset(ActiveKeys *keys);
bool      ActiveKeys__add_cell(ActiveKeys *keys, Cell cell);

BoundKey* ActiveKeys__first(ActiveKeys *keys);
BoundKey* ActiveKeys__next(ActiveKeys *keys);


#endif // __ACTIVE_KEYS_H__
