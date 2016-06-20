#include <time.h>
#include <stdlib.h>
#include "globals.h"
#include "hash.h"
#include "board.h"

u64 generate_hash(S_BOARD *b) {
    int sq;
    u64 hash_key;
    int pce;

    for(sq = 0; sq < 64; sq++) {
        pce = b->sq[sq];
        if(pce != EMPTY) {
            hash_key ^= pce_key[pce][sq];
        }
    }

    if(b->side != WHITE) {
        hash_key ^= side_key;
    }

    if (b->ep_sq != 0) {
        hash_key ^= pce_key[EMPTY][b->ep_sq];
    }

    assert(b->castle_perm >= 0 && b->castle_perm <= 15);
    hash_key ^= castle_key[b->castle_perm];

    return hash_key;
}

u64 rand64() {
    u64 rand64 = 0LL;

    rand64 = (0xff000000 & ((u64)rand() << 48));
    rand64 = (0x00ff0000 & ((u64)rand() << 32));
    rand64 = (0x0000ff00 & ((u64)rand() << 16));
    rand64 = (0x000000ff & (u64)rand());

    return rand64;
}

void init_hash() { //TODO init this
    srand(time(NULL));

    int i, j;

    side_key = rand64();

    for(i = 0; i < 16; i++) {
        castle_key[i] = rand64();
    }

    for(i = 0; i < 13; i++) {
        for(j = 0; j < 120; j++) {
            pce_key[i][j] = rand64();
        }
    }
}
