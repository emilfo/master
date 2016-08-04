#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>
//#include <string.h>
#include <assert.h>
#include <stdbool.h>

#ifndef DNDEBUG
#include "debug.h"
#endif

//int-types
#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8  uint8_t
#define i64 int64_t
#define i32 int32_t
#define i16 int16_t
#define i8  int8_t


#define NAME "Kholin Chess Engine"
#define AUTHOR "Emil F Ostensen"
#define QUOTE "\n\"Well, you've shown me something today . . . You've shown me that I'm still a threat. \n\t- Dalinar Kholin\n"

#define WHITE 0
#define BLACK 1
#define BOTH  2

#define MIN(a, b) (( a < b)? a : b )
#define MAX(a, b) (( a > b)? a : b )

#define MAX_PLY INT8_MAX
#define INFINITE INT16_MAX
#define MATE 30000
#define ISMATE (30000 - MAX_PLY)

#define mv_from(m) ((m) & (0b111111))
#define mv_to(m) ((m >> 6) & (0b111111))
#define mv_piece(m) ((m >> 12) & (0b1111))
#define mv_cap(m) ((m >> 16) & (0b1111))
#define mv_prom(m) ((m >> 20) & (0b1111))
#define mv_ep(m) ((m >> 25) & (0b1))
#define mv_castle(m) ((m >> 26) & (0b1))
#define mv_pawn_double(m) ((m >> 27) & (0b1))

#define FRtoSQ(file, rank) ((8*(rank-1)) + (file-1))

//UCI-options
#define HASH_DEF 64
#define HASH_MIN 1
#define HASH_MAX 1000
#define THREADS_DEF 1
#define THREADS_MIN 1
#define THREADS_MAX 64


const enum { WKCA=0b0001, WQCA=0b0010, BKCA=0b0100, BQCA=0b1000 } CASTLE_PERM;

const enum { EMPTY,
    W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING
} PIECES;

static const int PIECE_COLOR[13] = { -1,
    0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1
};

static const char PIECE_NAME[13] = { ' ',
    'P', 'N', 'B', 'R', 'Q', 'K',
    'p', 'n', 'b', 'r', 'q', 'k'
};

static const int PIECE_VAL[13] = { 0,
    100, 325, 325, 550, 1000, 20000,
    100, 325, 325, 550, 1000, 20000,
};

static const int KING_INDEX[2] = {
    6, 12
};

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

static const int files[64] = {
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
};

static const int ranks[64] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8,
};

#endif /* DEFS_H */
