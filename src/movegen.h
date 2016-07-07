#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"

const static int CAP_VAL[13] = { 0,
    100, 200, 300, 400, 500, 600,
    100, 200, 300, 400, 500, 600,
};

const static int ATT_VAL[13] = { 0,
    5, 4, 3, 2, 1, 0,
    5, 4, 3, 2, 1, 0,
};

int sq_attacked(const S_BOARD *b, u64 target_bb, int from_side); 

void generate_all_moves(const S_BOARD *b, S_MOVELIST *list); 
void generate_all_captures(const S_BOARD *b, S_MOVELIST *list);

#endif /* MOVEGEN_H */
