#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "defs.h"
#include "board.h"

#define AGE_MASK  0x0fff
#define FLAG_MASK 0xf000
#define BETA_FLAG 0x1000
#define ALPH_FLAG 0x2000
#define EXCA_FLAG 0x4000

typedef struct {
    u64 hash_key;
    u32 move;
    u32 checksum;
    i16 eval;
    u16 flag_and_age;
    u8  depth;
} S_HASHENTRY;

typedef struct {
    S_HASHENTRY *entries;
    int size;
    int cut;
    int fail_checksum;
} S_HASHTABLE;

void init_hashtable(u64 size);
void clear_hashtable();
void destroy_hashtable();
int hash_get(u64 key, S_HASHENTRY *entry);
void hash_put(u64 key, u32 move, i16 eval, u8 depth, i16 age, i16 flag);
int hash_get_pv_line(S_BOARD *b, u32 *moves, int depth);
int probe_hash(u64 key, S_HASHENTRY *entry, i16 *score, i16 alpha, i16 beta, int depth);

extern u64 tp_size; 

int fail_checksum;

#endif /* HASHTABLE_H */
