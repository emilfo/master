#ifndef BITOPS_H
#define BITOPS_H

#include "globals.h"

int bit_count(u64 bitmap);
int lsb1_index(u64 bitmap);
int msb1_index(u64 bitmap);

int MS1BTABLE[256];
#endif /* BITOPS_H */
