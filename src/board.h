#ifndef BOARD_H
#define BOARD_H

#include "defs.h"

typedef uint64_t u64;

#define C64 (x) xULL
#define U64FULL C64(0xFFFFFFFFFFFFFFFF)

#define MAX_MOVE_BUF 1024

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define TEST_FEN "rNb5/3P4/2P1n3/P4R2/4K2p/p1p3k1/5b2/nR6 w - - 0 1"
#define PAWNMOVES "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define PAWNMOVESB "rnbqkbnr/p1p1p3/3p3p/1p1p4/2P1Pp2/8/PP1P1PpP/RNBQKB1R b KQkq e3 0 1"
#define KNIGHTSKINGS "5k2/1n6/4n3/6N1/8/3N4/8/5K2 w - - 0 1"
#define ROOKS "6k1/8/5r2/8/1nR5/5N2/8/6K1 w - - 0 1"
#define QUEENS "6k1/8/4nq2/8/1nB5/5N2/1N6/6K1 w - - 0 1 "
#define BISHOPS "6k1/1b6/4n3/8/1n4B1/1B3N2/1N6/2b3K1 b - - 0 1 "
#define BISHOPSB "6K1/1B6/4N3/8/1N4b1/1b3n2/1n6/2B3k1 b - - 0 1 "
#define CASTLE1 "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"
#define CASTLE2 "3rk2r/8/8/8/8/8/6p1/R3K2R b KQk - 0 1"


typedef struct {
    int move;
    uint8_t castle_perm, ep_sq, fifty_move_count;
    u64 hash_key;
} S_PREV_BOARD;

typedef struct {
    u64 piece_bb[13];
    u64 all_piece_bb[3];

    uint8_t side, castle_perm, ep_sq, fifty_move_count;

    uint8_t sq[64];

    u64 hash_key;

    //used to unmake move
    S_PREV_BOARD prev[MAX_MOVE_BUF];
    int search_ply;
    i16 ply;

    u32 search_history[13][64];
    u32 search_killers[2][MAX_PLY];
    u32 principal_variation[MAX_PLY];

    long nodes;
    int fail_high;
    int first_fail_high;

} S_BOARD;

//Struct used for pretty printing only
typedef struct {
    uint8_t rank[8];
} BIT_BOARD;

//Masks
u64 RANK_MASK[64];
u64 FILE_MASK[64];
u64 DIAGA1H8_MASK[64];
u64 DIAGA8H1_MASK[64];
u64 FILE_MAGIC[64];
u64 DIAGA1H8_MAGIC[64];
u64 DIAGA8H1_MAGIC[64];

//castling
//u64 MASK_EG[2];
//u64 MASK_FG[2];
//u64 MASK_BD[2];
//u64 MASK_CE[2];
u64 OO_MASK[2];
u64 OOO_MASK[2];
u64 OO_ATTACK_MASK[2];
u64 OOO_ATTACK_MASK[2];

//Attack bitmaps
u64 KNIGHT_ATTACKS[64];
u64 KING_ATTACKS[64];
u64 WHITE_PAWN_ATTACKS[64];
u64 WHITE_PAWN_MOVES[64];
u64 WHITE_PAWN_DOUBLE_MOVES[64];
u64 BLACK_PAWN_ATTACKS[64];
u64 BLACK_PAWN_MOVES[64];
u64 BLACK_PAWN_DOUBLE_MOVES[64];   
u64 RANK_ATTACKS[64][64];
u64 FILE_ATTACKS[64][64];
u64 DIAGA8H1_ATTACKS[64][64];
u64 DIAGA1H8_ATTACKS[64][64];


unsigned char GEN_SLIDING_ATTACKS[8][64];

extern const u64 _DIAGA8H1MAGICS[];
extern const u64 _DIAGA1H8MAGICS[];
extern const u64 _FILEMAGICS[];
extern const int RANKSHIFT[];

void init_board();
int parse_fen(S_BOARD *b, char *fen);
void soft_reset_board(S_BOARD *b); 
void hard_reset_board(S_BOARD *b);
int make_move(S_BOARD *b, int);
void unmake_move(S_BOARD *b);
void make_null_move(S_BOARD *b);
void unmake_null_move(S_BOARD *b);
int make_move_if_exist(S_BOARD *b, u32 move);
void flip_board(S_BOARD *b);
void print_board(const S_BOARD *b);
int debug_board(S_BOARD *b);
void print_bitboard_rank(uint8_t rank);
void print_bitboard(BIT_BOARD  *bboard);

static const int mirror_piece[13] = { 0,
    7,  8,  9, 10, 11, 12,
    1,  2,  3,  4,  5,  6
};

#endif /* BOARD_H */
