#ifndef EVAL_H
#define EVAL_H

int eval_posistion(const S_BOARD *b);
void test();

u64 EVAL_RANK_MASK[8];
u64 EVAL_FILE_MASK[8];

u64 BLACK_PASSED_MASK[64];
u64 WHITE_PASSED_MASK[64];
u64 ISOLATED_PAWN_MASK[64];

extern const int mirror[64];

#endif /* EVAL_H */
