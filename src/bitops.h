#ifndef BITOPS_H
#define BITOPS_H

#include "defs.h"

#ifdef __GNUC__NO
#define lsb1_index(x) (__builtin_ffsll(x)-1)
#define bit_count(x) (__builtin_popcountll(x))
#else
#define lsb1_index(x) (lsb1_index_debruijn(x))
#define bit_count(x) (bit_count_knuth(x))
#endif

int bit_count_knuth(u64 bitmap);
int lsb1_index_debruijn(u64 bitmap);
int msb1_index(u64 bitmap);
u64 flipVertical(u64 x);

int MS1BTABLE[256];
#endif /* BITOPS_H */
