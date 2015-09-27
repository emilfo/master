#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

typedef uint64_t u64;

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef struct {
  u64 w_king, w_queens, w_rooks, w_bishops, w_knights, w_pawns,
      b_king, b_queens, b_rooks, b_bishops, b_knights, b_pawns,
      w_pieces, b_pieces, all_pieces;

  uint8_t side, castle_perm;

  int ep_sq, fifty_move_count;
  uint8_t sq[64];
} S_BOARD;

void init();
int parse_fen(char* fen);
void reset_board();
void print_board();


#endif
