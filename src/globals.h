#ifndef GLOBALS_H
#define GLOBALS_H

#include <assert.h>
#include <stdbool.h>

#include "board.h"
#include "bitops.h"
#include "debug.h"

#define PROG_VERSION "Chess 0.1, by Emil F Ostensen"
#define FRtoSQ(file, rank) ((8*(rank-1)) + (file-1))
#define WHITE 0
#define BLACK 1

S_BOARD board;


/* uint32_t move
 * 0000 0000 0000 0000 0000 0011 1111 -> From, 0x3F
 * 0000 0000 0000 0000 1111 1100 0000 -> To, 0x3F << 6
 * 0000 0000 0000 1111 0000 0000 0000 -> Piece, 0xF << 12
 * 0000 0000 1111 0000 0000 0000 0000 -> Captured, 0xF << 16
 * 0000 1111 0000 0000 0000 0000 0000 -> Promoted, 0xF << 20
 * 0001 0000 0000 0000 0000 0000 0000 -> EP, 0x1000000 (1 << 25)
 * 0010 0000 0000 0000 0000 0000 0000 -> Castling, 0x2000000 (1 << 26)
 */
#define mv_from(m) ((m) & (0b111111))
#define mv_to(m) ((m >> 6) & (0b111111))
#define mv_piece(m) ((m >> 12) & (0b1111))
#define mv_cap(m) ((m >> 16) & (0b1111))
#define mv_prom(m) ((m >> 20) & (0b1111))
#define mv_ep(m) ((m >> 25) & (0b1))
#define mv_castle(m) ((m >> 26) & (0b1))
#define mv_pawn_double(m) ((m >> 27) & (0b1))

const enum { WKCA=0b0001, WQCA=0b0010, BKCA=0b0100, BQCA=0b1000 } CASTLE_PERM;

const enum { EMPTY,
    W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING
} PIECES;

const static char PIECE_NAME[13] = { ' ',
    'P', 'N', 'B', 'R', 'Q', 'K',
    'p', 'n', 'b', 'r', 'q', 'k'
};

//const enum { WHITE, BLACK } SIDE;

const enum { NOCASTLING, OOCASTLE, OOOCASTLE } CASTLING;

const enum {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8
} SQUARES;

const static int files[64] = {
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
};

const static int ranks[64] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8,
};
//extern const int files[];
//extern const int ranks[];



#endif /* GLOBALS_H */
