#ifndef HASH_H
#define HASH_H

u64 side_key;
u64 castle_key[16];
u64 pce_key[13][120];

void init_rand();
u64 rand64();

u64 generate_hash(S_BOARD *b);

#endif /* HASH_H */
