#ifndef GLOBALS_H
#define GLOBALS_H

#include <assert.h>
#include "board.h"

#define FRtoSQ(file, rank) (8*(rank-1)) + (file-1)

S_BOARD board;

const enum { WKCA=0b0001, WQCA=0b0010, BKCA=0b0100, BQCA=0b1000 } CASTLE_PERM;

const enum { EMPTY,
  W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
  B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING
} PIECES;

const enum { WHITE, BLACK } SIDE;

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


//const char* SQ_NAME[64] = {
//  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
//  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
//  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
//  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
//  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
//  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
//  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
//  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
//};


#endif
