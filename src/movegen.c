#include <stdio.h>

#include "movegen.h"
#include "globals.h"
#include "board.h"
#include "bitops.h"

// Macro's to define sliding attacks:
//Bishop moves - square, occupied_bb, target_bb
#define SLIDEA8H1MOVES(sq, oc, ta) (DIAGA8H1_ATTACKS[sq][((oc & DIAGA8H1_MASK[sq]) * DIAGA8H1_MAGIC[sq]) >> 57] & ta)
#define SLIDEA1H8MOVES(sq, oc, ta) (DIAGA1H8_ATTACKS[sq][((oc & DIAGA1H8_MASK[sq]) * DIAGA1H8_MAGIC[sq]) >> 57] & ta)
#define BISHOPMOVES(sq, oc, ta) (SLIDEA8H1MOVES(sq, oc, ta) | SLIDEA1H8MOVES(sq, oc, ta))

//rookmoves - square, occupied_bb, target_bb
#define RANKMOVES(sq, oc, ta) (RANK_ATTACKS[sq][((oc & RANK_MASK[sq]) >> RANKSHIFT[sq])] & ta)
#define FILEMOVES(sq, oc, ta) (FILE_ATTACKS[sq][((oc & FILE_MASK[sq]) * FILE_MAGIC[sq]) >> 57] & ta)
#define ROOKMOVES(sq, oc, ta) (RANKMOVES(sq, oc, ta) | FILEMOVES(sq, oc, ta))

//queenmoves - square, occupied_bb, target_bb
#define QUEENMOVES(sq, oc, ta) (ROOKMOVES(sq, oc, ta) | BISHOPMOVES(sq, oc, ta))


static void generate_black_pawn_moves(const S_BOARD *b, S_MOVELIST *list, u64 free_sq_bb);
static void generate_white_pawn_moves(const S_BOARD *b, S_MOVELIST *list, u64 free_sq_bb);
static void generate_black_knight_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb);
static void generate_white_knight_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb);
static void generate_black_bishop_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb);
static void generate_white_bishop_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb);
static void generate_black_rook_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb);
static void generate_white_rook_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb);
static void generate_black_queen_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb);
static void generate_white_queen_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb);
static void generate_black_king_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb);
static void generate_white_king_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb);
static void add_move(const S_BOARD *b, S_MOVELIST *list, int from, int to, int piece, int capture, int promoted, int ep, int castling);
static void generate_black_pawn_captures(const S_BOARD *b, S_MOVELIST *list);
static void generate_white_pawn_captures(const S_BOARD *b, S_MOVELIST *list);


/* move
 * 0000 0000 0000 0000 0000 0011 1111 -> From, 0x3F
 * 0000 0000 0000 0000 1111 1100 0000 -> To, 0x3F << 6
 * 0000 0000 0000 1111 0000 0000 0000 -> Piece, 0xF << 12
 * 0000 0000 1111 0000 0000 0000 0000 -> Captured, 0xF << 16
 * 0000 1111 0000 0000 0000 0000 0000 -> Promoted, 0xF << 20
 * 0001 0000 0000 0000 0000 0000 0000 -> EP, 0x1000000 (1 << 25)
 * 0010 0000 0000 0000 0000 0000 0000 -> Castling, 0x2000000 (1 << 26)
 */
//generate the move from, to, piece, capture, promoted, ep, castling
#define gen_move(f, t, p, c, pr, ep, ca) ((f) | (t<<6) | (p<<12) | (c<<16) | (pr<<20) | (ep<<25) | (ca<<26))
//#define from_sq(m) ((m) & (0b111111))
//#define to_sq(m) ((m >> 6) & (0b111111))
//#define piece(m) ((m >> 12) & (0b1111))
//#define cap(m) ((m >> 16) & (0b1111))
//#define prom(m) ((m >> 20) & (0b1111))
//#define ep(m) ((m >> 25) & (0b1))
//#define castling(m) ((m >> 26) & (0b1))

void generate_all_moves(const S_BOARD *b, S_MOVELIST *list) 
{
    u64 targets_bb, free_sq_bb;

    list->index = 0;
    list->returned = 0;
    free_sq_bb = ~(b->all_piece_bb[BOTH]);

    if(b->side) { //black to move
        targets_bb = ~(b->all_piece_bb[BLACK]);

        generate_black_pawn_moves(b, list, free_sq_bb);
        generate_black_knight_moves(b, list, targets_bb);
        generate_black_bishop_moves(b, list, targets_bb);
        generate_black_rook_moves(b, list, targets_bb);
        generate_black_queen_moves(b, list, targets_bb);
        generate_black_king_moves(b, list, targets_bb);

    } else { //white to move
        targets_bb = ~(b->all_piece_bb[WHITE]);

        generate_white_pawn_moves(b, list, free_sq_bb);
        generate_white_knight_moves(b, list, targets_bb);
        generate_white_bishop_moves(b, list, targets_bb);
        generate_white_rook_moves(b, list, targets_bb);
        generate_white_queen_moves(b, list, targets_bb);
        generate_white_king_moves(b, list, targets_bb);
    }
}

void generate_all_captures(const S_BOARD *b, S_MOVELIST *list) 
{
    u64 targets_bb;

    list->index = 0;
    list->returned = 0;

    if(b->side) { //black to move
        targets_bb = b->all_piece_bb[WHITE];

        generate_black_pawn_captures(b, list);
        generate_black_knight_moves(b, list, targets_bb);
        generate_black_bishop_moves(b, list, targets_bb);
        generate_black_rook_moves(b, list, targets_bb);
        generate_black_queen_moves(b, list, targets_bb);
        generate_black_king_moves(b, list, targets_bb);

    } else { //white to move
        targets_bb = b->all_piece_bb[BLACK];

        generate_white_pawn_captures(b, list);
        generate_white_knight_moves(b, list, targets_bb);
        generate_white_bishop_moves(b, list, targets_bb);
        generate_white_rook_moves(b, list, targets_bb);
        generate_white_queen_moves(b, list, targets_bb);
        generate_white_king_moves(b, list, targets_bb);
    }
}
/** 
 * Runs through the bitboard of the black pawns (cur_pce_bb), for each pawn it
 * ticks of possible moves it can do in a new bitboard (cur_move_bb).  For each
 * cur_move_bb it run through all the the ticked of sq and add each generated
 * move to the list of moves
 */
static void generate_black_pawn_moves(const S_BOARD *b, S_MOVELIST *list, u64 free_sq_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b->piece_bb[B_PAWN];
    while(cur_pce_bb) {

        from = lsb1_index(cur_pce_bb);

        //tick off moves that the pawn can take
        cur_move_bb = BLACK_PAWN_MOVES[from] & free_sq_bb;

        if (cur_move_bb && ranks[from] == 7) {
            cur_move_bb |= BLACK_PAWN_DOUBLE_MOVES[from] & free_sq_bb;
        }
        cur_move_bb |= BLACK_PAWN_ATTACKS[from] & b->all_piece_bb[WHITE];

        while(cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b->sq[to];

            if (ranks[to] == 1) {
                add_move(b, list, from, to, B_PAWN, capture, B_KNIGHT, false, false);
                add_move(b, list, from, to, B_PAWN, capture, B_BISHOP, false, false);
                add_move(b, list, from, to, B_PAWN, capture, B_ROOK, false, false);
                add_move(b, list, from, to, B_PAWN, capture, B_QUEEN, false, false);
            } else {
                add_move(b, list, from, to, B_PAWN, capture, false, false, false);
            }

            cur_move_bb ^= (1LL << to);
        }

        if(b->ep_sq) {
            if(BLACK_PAWN_ATTACKS[from] & (1LL << b->ep_sq)) {
                add_move(b, list, from, b->ep_sq, B_PAWN, W_PAWN, false, true, false);
            }
        }

        cur_pce_bb ^= (1LL << from);
    }
}

/** 
 * Runs through the bitboard of the white pawns (cur_pce_bb), for each pawn it
 * ticks of possible moves it can do in a new bitboard (cur_move_bb).  For each
 * cur_move_bb it run through all the the ticked of sq and add each generated
 * move to the *list of moves
 */
static void generate_white_pawn_moves(const S_BOARD *b, S_MOVELIST *list, u64 free_sq_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b->piece_bb[W_PAWN];
    while(cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        //tick off moves that the pawn can take
        cur_move_bb = WHITE_PAWN_MOVES[from] & free_sq_bb;

        if (cur_move_bb && ranks[from] == 2) {
            cur_move_bb |= WHITE_PAWN_DOUBLE_MOVES[from] & free_sq_bb;
        }
        cur_move_bb |= WHITE_PAWN_ATTACKS[from] & b->all_piece_bb[BLACK];

        while(cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b->sq[to];

            if (ranks[to] == 8) {
                add_move(b, list, from, to, W_PAWN, capture, W_KNIGHT, false, false);
                add_move(b, list, from, to, W_PAWN, capture, W_BISHOP, false, false);
                add_move(b, list, from, to, W_PAWN, capture, W_ROOK, false, false);
                add_move(b, list, from, to, W_PAWN, capture, W_QUEEN, false, false);
            } else {
                add_move(b, list, from, to, W_PAWN, capture, false, false, false);
            }

            cur_move_bb ^= (1LL << to);
        }

        if(b->ep_sq) {
            if(WHITE_PAWN_ATTACKS[from] & (1LL << b->ep_sq)) {
                add_move(b, list, from, b->ep_sq, W_PAWN, B_PAWN, false, true, false);
            }
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_black_knight_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b->piece_bb[B_KNIGHT];
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = KNIGHT_ATTACKS[from] & targets_bb;
        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b->sq[to];
            add_move(b, list, from, to, B_KNIGHT, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_white_knight_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b->piece_bb[W_KNIGHT];
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = KNIGHT_ATTACKS[from] & targets_bb;
        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b->sq[to];
            add_move(b, list, from, to, W_KNIGHT, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_black_bishop_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b->piece_bb[B_BISHOP];
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = BISHOPMOVES(from, b->all_piece_bb[BOTH], targets_bb);
        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b->sq[to];
            add_move(b, list, from, to, B_BISHOP, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_white_bishop_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b->piece_bb[W_BISHOP];
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = BISHOPMOVES(from, b->all_piece_bb[BOTH], targets_bb);

        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b->sq[to];
            add_move(b, list, from, to, W_BISHOP, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_black_rook_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b->piece_bb[B_ROOK];
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = ROOKMOVES(from, b->all_piece_bb[BOTH], targets_bb);
        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b->sq[to];
            add_move(b, list, from, to, B_ROOK, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_white_rook_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b->piece_bb[W_ROOK];
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = ROOKMOVES(from, b->all_piece_bb[BOTH], targets_bb);
        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b->sq[to];
            add_move(b, list, from, to, W_ROOK, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_black_queen_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b->piece_bb[B_QUEEN];
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = QUEENMOVES(from, b->all_piece_bb[BOTH], targets_bb);
        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b->sq[to];
            add_move(b, list, from, to, B_QUEEN, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_white_queen_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b->piece_bb[W_QUEEN];
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = QUEENMOVES(from, b->all_piece_bb[BOTH], targets_bb);
        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b->sq[to];
            add_move(b, list, from, to, W_QUEEN, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_black_king_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_move_bb;

    from = lsb1_index(b->piece_bb[B_KING]);//We can assume the king is always on the board

    cur_move_bb = KING_ATTACKS[from] & targets_bb;
    while (cur_move_bb) {
        to = lsb1_index(cur_move_bb);

        capture = b->sq[to];
        add_move(b, list, from, to, B_KING, capture, false, false, false);

        cur_move_bb ^= (1LL << to);
    }

    //Castling
    if ((b->castle_perm & BKCA) && !(b->all_piece_bb[BOTH] & OO_MASK[BLACK])) {
        if (!sq_attacked(b, OO_ATTACK_MASK[BLACK], WHITE)) {
            add_move(b, list, from, G8, B_KING, false, false, false, true);
        }
    }
    if ((b->castle_perm & BQCA) && !(b->all_piece_bb[BOTH] & OOO_MASK[BLACK])) {
        if (!sq_attacked(b, OOO_ATTACK_MASK[BLACK], WHITE)) {
            add_move(b, list, from, C8, B_KING, false, false, false, true);
        }
    }
}

static void generate_white_king_moves(const S_BOARD *b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_move_bb;

    from = lsb1_index(b->piece_bb[W_KING]);//We can assume the king is always on the board

    cur_move_bb = KING_ATTACKS[from] & targets_bb;
    while (cur_move_bb) {
        to = lsb1_index(cur_move_bb);

        capture = b->sq[to];
        add_move(b, list, from, to, W_KING, capture, false, false, false);

        cur_move_bb ^= (1LL << to);
    }

    //Castling
    if ((b->castle_perm & WKCA) && !(b->all_piece_bb[BOTH] & OO_MASK[WHITE])) {
        if (!sq_attacked(b, OO_ATTACK_MASK[WHITE], BLACK)) {
            add_move(b, list, from, G1, W_KING, false, false, false, true);
        }
    }
    if ((b->castle_perm & WQCA) && !(b->all_piece_bb[BOTH] & OOO_MASK[WHITE])) {
        if (!sq_attacked(b, OOO_ATTACK_MASK[WHITE], BLACK)) {
            add_move(b, list, from, C1, W_KING, false, false, false, true);
        }
    }
}

/**
 * Used to see if any square in a bitmap is under target.
 * smart to use both to see if king is in check, but also check if castling is
 * ok
 */
int sq_attacked(const S_BOARD *b, u64 target_bb, int from_side) 
{
    u64 slide_attacks_bb;
    int sq;

    if (from_side == BLACK) {
        while (target_bb) {
            sq = lsb1_index(target_bb);

            if (b->piece_bb[B_PAWN] & WHITE_PAWN_ATTACKS[sq]) return true;
            if (b->piece_bb[B_KNIGHT] & KNIGHT_ATTACKS[sq]) return true;
            if (b->piece_bb[B_KING] & KING_ATTACKS[sq]) return true;

            slide_attacks_bb = b->piece_bb[B_QUEEN] | b->piece_bb[B_ROOK];
            if (slide_attacks_bb) {
                if (RANK_ATTACKS[sq][(b->all_piece_bb[BOTH] & RANK_MASK[sq]) >> RANKSHIFT[sq]]
                        & slide_attacks_bb) { 
                    return true; 
                }
                if (FILE_ATTACKS[sq][((b->all_piece_bb[BOTH] & FILE_MASK[sq]) *
                            FILE_MAGIC[sq]) >> 57] & slide_attacks_bb) { 
                    return true; 
                }
            }

            slide_attacks_bb = b->piece_bb[B_QUEEN] | b->piece_bb[B_BISHOP];
            if (slide_attacks_bb) {
                if (DIAGA8H1_ATTACKS[sq][((b->all_piece_bb[BOTH] & DIAGA8H1_MASK[sq]) 
                            * DIAGA8H1_MAGIC[sq]) >> 57] & slide_attacks_bb) {
                    return true;
                }
                if (DIAGA1H8_ATTACKS[sq][((b->all_piece_bb[BOTH] & DIAGA1H8_MASK[sq])
                            * DIAGA1H8_MAGIC[sq]) >> 57] & slide_attacks_bb) {
                    return true;
                }

            }
            target_bb ^= (1LL << sq);
        }

    } else {//SIDE == WHITE
        while (target_bb) {
            sq = lsb1_index(target_bb);

            if (b->piece_bb[W_PAWN] & BLACK_PAWN_ATTACKS[sq]) return true;
            if (b->piece_bb[W_KNIGHT] & KNIGHT_ATTACKS[sq]) return true;
            if (b->piece_bb[W_KING] & KING_ATTACKS[sq]) return true;

            slide_attacks_bb = b->piece_bb[W_QUEEN] | b->piece_bb[W_ROOK];
            if (slide_attacks_bb) {
                if (RANK_ATTACKS[sq][(b->all_piece_bb[BOTH] & RANK_MASK[sq]) >> RANKSHIFT[sq]]
                        & slide_attacks_bb) { 
                    return true; 
                }
                if (FILE_ATTACKS[sq][((b->all_piece_bb[BOTH] & FILE_MASK[sq]) *
                            FILE_MAGIC[sq]) >> 57] & slide_attacks_bb) { 
                    return true; 
                }
            }

            slide_attacks_bb = b->piece_bb[W_QUEEN] | b->piece_bb[W_BISHOP];
            if (slide_attacks_bb) {
                if (DIAGA8H1_ATTACKS[sq][((b->all_piece_bb[BOTH] & DIAGA8H1_MASK[sq]) 
                            * DIAGA8H1_MAGIC[sq]) >> 57] & slide_attacks_bb) {
                    return true;
                }
                if (DIAGA1H8_ATTACKS[sq][((b->all_piece_bb[BOTH] & DIAGA1H8_MASK[sq])
                            * DIAGA1H8_MAGIC[sq]) >> 57] & slide_attacks_bb) {
                    return true;
                }
            }
            target_bb ^= (1LL << sq);
        }
    }

    return false;
}

static void add_move(const S_BOARD *b, S_MOVELIST *list, int from, int to, int piece, int capture, int promoted, int ep, int castling)
{
    assert(valid_sq(from));
    assert(valid_sq(to));
    assert(valid_piece(piece));
    assert(valid_piece_or_empty(capture));
    assert(valid_piece_or_empty(promoted));
    assert(valid_bool(ep));
    assert(valid_bool(castling));

    list->moves[list->index].move = gen_move(from, to, piece, capture, promoted, ep, castling);

    if(capture) {
        list->moves[list->index].score = CAP_VAL[capture] + ATT_VAL[piece] + 1000000;
    } else if (global_search_killers[0][b->ply] == list->moves[list->index].move) {
        list->moves[list->index].score = 900000;
    } else if (global_search_killers[1][b->ply] == list->moves[list->index].move) {
        list->moves[list->index].score = 800000;
    } else {
        list->moves[list->index].score = global_search_history[piece][to];
    }

    list->index++;
}

static void generate_black_pawn_captures(const S_BOARD *b, S_MOVELIST *list) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b->piece_bb[B_PAWN];
    while(cur_pce_bb) {

        from = lsb1_index(cur_pce_bb);

        cur_move_bb = BLACK_PAWN_ATTACKS[from] & b->all_piece_bb[WHITE];

        while(cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b->sq[to];

            if (ranks[to] == 1) {
                add_move(b, list, from, to, B_PAWN, capture, B_KNIGHT, false, false);
                add_move(b, list, from, to, B_PAWN, capture, B_BISHOP, false, false);
                add_move(b, list, from, to, B_PAWN, capture, B_ROOK, false, false);
                add_move(b, list, from, to, B_PAWN, capture, B_QUEEN, false, false);
            } else {
                add_move(b, list, from, to, B_PAWN, capture, false, false, false);
            }

            cur_move_bb ^= (1LL << to);
        }

        if(b->ep_sq) {
            if(BLACK_PAWN_ATTACKS[from] & (1LL << b->ep_sq)) {
                add_move(b, list, from, b->ep_sq, B_PAWN, W_PAWN, false, true, false);
            }
        }

        cur_pce_bb ^= (1LL << from);
    }
}

/** 
 * Runs through the bitboard of the white pawns (cur_pce_bb), for each pawn it
 * ticks of possible moves it can do in a new bitboard (cur_move_bb).  For each
 * cur_move_bb it run through all the the ticked of sq and add each generated
 * move to the *list of moves
 */
static void generate_white_pawn_captures(const S_BOARD *b, S_MOVELIST *list) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b->piece_bb[W_PAWN];
    while(cur_pce_bb) {

        from = lsb1_index(cur_pce_bb);

        cur_move_bb = WHITE_PAWN_ATTACKS[from] & b->all_piece_bb[BLACK];

        while(cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b->sq[to];

            if (ranks[to] == 8) {
                add_move(b, list, from, to, W_PAWN, capture, W_KNIGHT, false, false);
                add_move(b, list, from, to, W_PAWN, capture, W_BISHOP, false, false);
                add_move(b, list, from, to, W_PAWN, capture, W_ROOK, false, false);
                add_move(b, list, from, to, W_PAWN, capture, W_QUEEN, false, false);
            } else {
                add_move(b, list, from, to, W_PAWN, capture, false, false, false);
            }

            cur_move_bb ^= (1LL << to);
        }

        if(b->ep_sq) {
            if(WHITE_PAWN_ATTACKS[from] & (1LL << b->ep_sq)) {
                add_move(b, list, from, b->ep_sq, W_PAWN, B_PAWN, false, true, false);
            }
        }

        cur_pce_bb ^= (1LL << from);
    }
}
