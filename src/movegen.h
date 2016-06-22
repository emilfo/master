#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "globals.h"

#define MAX_MOVES 256
#define MAX_DEPTH 1024

typedef struct {
    uint32_t move;
    uint32_t score;
} S_MOVE;

typedef struct {
    S_MOVE moves[MAX_MOVES];
    int index;
} S_MOVELIST;

//Struct used for pretty printing only
typedef struct {
    uint32_t moves[MAX_MOVES];
    uint8_t index;
} S_MOVE_LIST;

//static int sq_attacked(S_BOARD b, u64 target_bb, int from_side); 

void generate_all_moves(S_BOARD b, S_MOVELIST *list); 

#endif /* MOVEGEN_H */
