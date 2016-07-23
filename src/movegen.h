#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "defs.h"
#include "board.h"

#define MAX_MOVES 256
#define MAX_DEPTH 1024

typedef struct {
    u32 move;
    int score;
} S_MOVE;

typedef struct {
    S_MOVE moves[MAX_MOVES];
    int index;
} S_MOVELIST;


static const int CAP_VAL[13] = { 0,
    100, 200, 300, 400, 500, 600,
    100, 200, 300, 400, 500, 600,
};

static const int ATT_VAL[13] = { 0,
    5, 4, 3, 2, 1, 0,
    5, 4, 3, 2, 1, 0,
};

int sq_attacked(const S_BOARD *b, u64 target_bb, int from_side); 

void generate_all_moves(const S_BOARD *b, S_MOVELIST *list); 
void generate_all_captures(const S_BOARD *b, S_MOVELIST *list);

#endif /* MOVEGEN_H */
