#ifndef MOVEGEN_H
#define MOVEGEN_H

static int sq_attacked(S_BOARD b, u64 target_bb, int from_side); 
void generate_all_moves(S_BOARD b, S_MOVE_LIST list); 

#endif /* MOVEGEN_H */
