#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "board.h"

#define AGE_MASK  0x0fff
#define FLAG_MASK 0xf000
#define BETA_FLAG 0x1000
#define ALPH_FLAG 0x2000
#define EXCA_FLAG 0x4000

typedef struct {
    u64 hash_key;
    uint32_t move;
    uint32_t checksum;
    int16_t eval;
    uint16_t flag_and_age;
    uint8_t depth;
} S_HASHENTRY;

typedef struct {
    S_HASHENTRY *entries;
    int size;
    int cut;
} S_HASHTABLE;

void init_hashtable(S_HASHTABLE *tp, u64 size);
void destroy_hashtable(S_HASHTABLE *tp);
int hash_get(const S_HASHTABLE *tp, u64 key, S_HASHENTRY *entry);
void hash_put(S_HASHTABLE *tp, u64 key, uint32_t move, int16_t eval, uint8_t depth, int16_t age, int16_t flag);
int hash_get_pv_line(const S_HASHTABLE *tp, S_BOARD *b, int *moves, int depth);
int probe_hash(const S_HASHTABLE *tp, u64 key, S_HASHENTRY *entry, int *score, int alpha, int beta, int depth);

extern u64 tp_size; 

int fail_checksum;

#endif /* HASHTABLE_H */
