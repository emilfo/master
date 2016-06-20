#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

typedef uint64_t u64;

#define C64 (x) xULL
#define U64FULL C64(0xFFFFFFFFFFFFFFFF)

#define MAX_MOVE_BUF 1024
#define MAX_PLY 256

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define TEST_FEN "rNb5/3P4/2P1n3/P4R2/4K2p/p1p3k1/5b2/nR6 w - - 0 1"

typedef struct {
    int move;
    uint8_t castle_perm, ep_sq, fifty_move_count;
    u64 hash_key;
} S_PREV_BOARD;

typedef struct {
    u64 w_king, w_queens, w_rooks, w_bishops, w_knights, w_pawns,
        b_king, b_queens, b_rooks, b_bishops, b_knights, b_pawns,
        w_pieces, b_pieces, all_pieces;

    uint8_t side, castle_perm, ep_sq, fifty_move_count;

    uint8_t sq[64];

    u64 hash_key;

    //used to unmake move
    S_PREV_BOARD prev[MAX_MOVE_BUF];
    int ply;

    //storing moves 
    uint32_t move_buffer[MAX_MOVE_BUF]; //all generated moves in current tree
    int move_buffer_len[MAX_PLY]; //which moves belongs to which ply, TODO:better way?
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

void init();
int parse_fen(char* fen);
void reset_board();
int make_move(int move, S_BOARD b);
void unmake_move(S_BOARD b);
void remove_piece(S_BOARD b, int piece, int sq);
void add_piece(S_BOARD b, int piece, int sq);
void print_board();
int debug_board();
void print_bitboard_rank(uint8_t rank);
void print_bitboard(BIT_BOARD  *bboard);

#endif /* BOARD_H */
