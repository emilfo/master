#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "board.h"

typedef struct {
    u64 hash_key;
    uint32_t move;
    int16_t eval;
    int16_t age;
} S_HASHENTRY;

typedef struct {
    S_HASHENTRY *entries;
    int size;
} S_HASHTABLE;

void init_hashtable(S_HASHTABLE *tp, u64 size);
S_HASHENTRY *hash_get(S_HASHTABLE *tp, u64 key);
void hash_put(S_HASHTABLE *tp, u64 key, uint32_t move, int16_t eval, int16_t age);
int hash_get_pv_line(S_HASHTABLE *tp, S_BOARD *b, int *moves, int depth);

extern u64 tp_size; 

#endif /* HASHTABLE_H */
