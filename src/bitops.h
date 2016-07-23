#ifndef BITOPS_H
#define BITOPS_H

#include "defs.h"

int bit_count(u64 bitmap);
int lsb1_index(u64 bitmap);
int msb1_index(u64 bitmap);
u64 flipVertical(u64 x);

int MS1BTABLE[256];
#endif /* BITOPS_H */
