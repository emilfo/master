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


static void generate_black_pawn_moves(S_BOARD b, S_MOVELIST *list, u64 free_sq_bb);
static void generate_white_pawn_moves(S_BOARD b, S_MOVELIST *list, u64 free_sq_bb);
static void generate_black_knight_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb);
static void generate_white_knight_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb);
static void generate_black_bishop_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb);
static void generate_white_bishop_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb);
static void generate_black_rook_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb);
static void generate_white_rook_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb);
static void generate_black_queen_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb);
static void generate_white_queen_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb);
static void generate_black_king_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb);
static void generate_white_king_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb);
static int sq_attacked(S_BOARD b, u64 target_bb, int from_side);
static void add_move(S_MOVELIST *list, int from, int to, int piece, int capture, int promoted, int ep, int castling);


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

void generate_all_moves(S_BOARD b, S_MOVELIST *list) 
{
    uint8_t opponent_side;
    //uint32_t to, from, capture;
    //u64 cur_pce_bb, cur_move_bb;
    u64 targets_bb, free_sq_bb;

    list->index = 0;
    opponent_side = !(b.side);
    free_sq_bb = ~(b.all_pieces);

    if(b.side) { //black to move
        targets_bb = ~(b.b_pieces);

        generate_black_pawn_moves(b, list, free_sq_bb);
        generate_black_knight_moves(b, list, targets_bb);
        generate_black_bishop_moves(b, list, targets_bb);
        generate_black_rook_moves(b, list, targets_bb);
        generate_black_queen_moves(b, list, targets_bb);
        generate_black_king_moves(b, list, targets_bb);

    } else { //white to move
        targets_bb = ~(b.w_pieces);

        generate_white_pawn_moves(b, list, free_sq_bb);
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
static void generate_black_pawn_moves(S_BOARD b, S_MOVELIST *list, u64 free_sq_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b.b_pawns;
    while(cur_pce_bb) {

        from = lsb1_index(cur_pce_bb);

        //tick off moves that the pawn can take
        cur_move_bb = BLACK_PAWN_MOVES[from] & free_sq_bb;

        if (cur_move_bb && ranks[from] == 7) {
            cur_move_bb |= BLACK_PAWN_DOUBLE_MOVES[from] & free_sq_bb;
        }
        cur_move_bb |= BLACK_PAWN_ATTACKS[from] & b.w_pieces;

        while(cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b.sq[to];

            if (ranks[to] == 1) {
                add_move(list, from, to, B_PAWN, capture, B_KNIGHT, false, false);
                add_move(list, from, to, B_PAWN, capture, B_BISHOP, false, false);
                add_move(list, from, to, B_PAWN, capture, B_ROOK, false, false);
                add_move(list, from, to, B_PAWN, capture, B_QUEEN, false, false);
            } else {
                add_move(list, from, to, B_PAWN, false, false, false, false);
            }

            cur_move_bb ^= (1LL << to);
        }

        if(board.ep_sq) {
            if(BLACK_PAWN_ATTACKS[from] & (1LL << board.ep_sq)) {
                add_move(list, from, board.ep_sq, B_PAWN, W_PAWN, false, true, false);
            }
        }

        cur_pce_bb ^= (1LL << from);
    }
}
//static void generate_black_pawn_moves(S_BOARD b, S_MOVELIST *list, u64 free_sq_bb) 
//{
//    printf("generating black pawn moves\n");
//    uint32_t from, to, capture;
//    u64 cur_pce_bb, cur_move_bb;
//
//    cur_pce_bb = b.b_pawns;
//    while(cur_pce_bb) {
//
//        from = lsb1_index(cur_pce_bb);
//
//        //tick off moves that the pawn can take
//        cur_move_bb = BLACK_PAWN_MOVES[from] & free_sq_bb;
//
//        if (cur_move_bb && ranks[from] == 7) {
//            cur_move_bb |= BLACK_PAWN_DOUBLE_MOVES[from] & free_sq_bb;
//        }
//        cur_move_bb |= BLACK_PAWN_ATTACKS[from] & b.w_pieces;
//
//        for (to = lsb1_index(cur_move_bb); cur_move_bb; cur_move_bb ^= (1LL << to)) {
//            //to = lsb1_index(cur_pce_bb);
//
//            capture = b.sq[to];
//
//            if (ranks[to] == 1) {
//                add_move(list, from, to, B_PAWN, capture, B_KNIGHT, false, false);
//                add_move(list, from, to, B_PAWN, capture, B_BISHOP, false, false);
//                add_move(list, from, to, B_PAWN, capture, B_ROOK, false, false);
//                add_move(list, from, to, B_PAWN, capture, B_QUEEN, false, false);
//            } else {
//                add_move(list, from, to, B_PAWN, false, false, false, false);
//            }
//
//            //cur_move_bb ^= (1LL << to);
//        }
//
//        if(board.ep_sq) {
//            if(BLACK_PAWN_ATTACKS[from] & (1LL << board.ep_sq)) {
//                add_move(list, from, to, B_PAWN, W_PAWN, false, true, false);
//            }
//        }
//
//        cur_pce_bb ^= (1LL << from);
//    }
//}

/** 
 * Runs through the bitboard of the white pawns (cur_pce_bb), for each pawn it
 * ticks of possible moves it can do in a new bitboard (cur_move_bb).  For each
 * cur_move_bb it run through all the the ticked of sq and add each generated
 * move to the *list of moves
 */
static void generate_white_pawn_moves(S_BOARD b, S_MOVELIST *list, u64 free_sq_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b.w_pawns;
    while(cur_pce_bb) {

        //printf("\n\ncurrently finding from this board:\n");
        //print_bitboard((BIT_BOARD *) &cur_pce_bb);

        from = lsb1_index(cur_pce_bb);

        //tick off moves that the pawn can take
        cur_move_bb = WHITE_PAWN_MOVES[from] & free_sq_bb;

        //printf("\nadding these moves:\n");
        //print_bitboard((BIT_BOARD *) &cur_move_bb);
        //print_bitboard((BIT_BOARD *) &WHITE_PAWN_MOVES[from]);
        //print_bitboard((BIT_BOARD *) &free_sq_bb);

        if (cur_move_bb && ranks[from] == 2) {
            cur_move_bb |= WHITE_PAWN_DOUBLE_MOVES[from] & free_sq_bb;
        }
        cur_move_bb |= WHITE_PAWN_ATTACKS[from] & b.b_pieces;

        while(cur_move_bb) {
            //printf("\nadding these moves:\n");
            //print_bitboard((BIT_BOARD *) &cur_move_bb);
            to = lsb1_index(cur_move_bb);

            capture = b.sq[to];

            if (ranks[to] == 8) {
                add_move(list, from, to, W_PAWN, capture, W_KNIGHT, false, false);
                add_move(list, from, to, W_PAWN, capture, W_BISHOP, false, false);
                add_move(list, from, to, W_PAWN, capture, W_ROOK, false, false);
                add_move(list, from, to, W_PAWN, capture, W_QUEEN, false, false);
            } else {
                add_move(list, from, to, W_PAWN, false, false, false, false);
            }

            cur_move_bb ^= (1LL << to);
        }

        if(board.ep_sq) {
            if(WHITE_PAWN_ATTACKS[from] & (1LL << board.ep_sq)) {
                add_move(list, from, board.ep_sq, W_PAWN, B_PAWN, false, true, false);
            }
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_black_knight_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b.b_knights;
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = KNIGHT_ATTACKS[from] & targets_bb;
        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b.sq[to];
            add_move(list, from, to, B_KNIGHT, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_white_knight_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b.w_knights;
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = KNIGHT_ATTACKS[from] & targets_bb;
        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b.sq[to];
            add_move(list, from, to, W_KNIGHT, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_black_bishop_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b.b_bishops;
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = BISHOPMOVES(from, b.all_pieces, targets_bb);
        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b.sq[to];
            add_move(list, from, to, B_BISHOP, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_white_bishop_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b.w_bishops;
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        printf("\nFrom these pieces:\n");
        print_bitboard((BIT_BOARD *) &cur_pce_bb);

        cur_move_bb = BISHOPMOVES(from, b.all_pieces, targets_bb);

        printf("\n\nUSING THESE VALS:\n\n");
        printf("all pieces:\n");
        print_bitboard((BIT_BOARD *) &(b.all_pieces));
        printf("targets_bb:\n");
        print_bitboard((BIT_BOARD *) &(targets_bb));

        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            printf("\nadding these moves:\n");
            print_bitboard((BIT_BOARD *) &cur_move_bb);

            capture = b.sq[to];
            add_move(list, from, to, W_BISHOP, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_black_rook_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b.b_rooks;
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = ROOKMOVES(from, b.all_pieces, targets_bb);
        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b.sq[to];
            add_move(list, from, to, B_ROOK, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_white_rook_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b.w_rooks;
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = ROOKMOVES(from, b.all_pieces, targets_bb);
        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b.sq[to];
            add_move(list, from, to, W_ROOK, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_black_queen_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b.b_queens;
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = QUEENMOVES(from, b.all_pieces, targets_bb);
        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b.sq[to];
            add_move(list, from, to, B_QUEEN, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_white_queen_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_pce_bb, cur_move_bb;

    cur_pce_bb = b.w_queens;
    while (cur_pce_bb) {
        from = lsb1_index(cur_pce_bb);

        cur_move_bb = QUEENMOVES(from, b.all_pieces, targets_bb);
        while (cur_move_bb) {
            to = lsb1_index(cur_move_bb);

            capture = b.sq[to];
            add_move(list, from, to, W_QUEEN, capture, false, false, false);

            cur_move_bb ^= (1LL << to);
        }

        cur_pce_bb ^= (1LL << from);
    }
}

static void generate_black_king_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_move_bb;

    from = lsb1_index(b.b_king);//We can assume the king is always on the board

    cur_move_bb = KING_ATTACKS[from] & targets_bb;
    while (cur_move_bb) {
        to = lsb1_index(cur_move_bb);

        capture = b.sq[to];
        add_move(list, from, to, B_KING, capture, false, false, false);

        cur_move_bb ^= (1LL << to);
    }

    //Castling
    if ((b.castle_perm & BKCA) && !(b.all_pieces & OO_MASK[BLACK])) {
        if (!sq_attacked(b, OO_ATTACK_MASK[BLACK], WHITE)) {
            add_move(list, from, G8, B_KING, false, false, false, true);
        }
    }
    if ((b.castle_perm & BQCA) && !(b.all_pieces & OOO_MASK[BLACK])) {
        if (!sq_attacked(b, OOO_ATTACK_MASK[BLACK], WHITE)) {
            add_move(list, from, C8, B_KING, false, false, false, true);
        }
    }
}

static void generate_white_king_moves(S_BOARD b, S_MOVELIST *list, u64 targets_bb) 
{
    uint32_t from, to, capture;
    u64 cur_move_bb;

    from = lsb1_index(b.w_king);//We can assume the king is always on the board

    cur_move_bb = KING_ATTACKS[from] & targets_bb;
    while (cur_move_bb) {
        to = lsb1_index(cur_move_bb);

        capture = b.sq[to];
        add_move(list, from, to, W_KING, capture, false, false, false);

        cur_move_bb ^= (1LL << to);
    }

    //Castling
    if ((b.castle_perm & WKCA) && !(b.all_pieces & OO_MASK[WHITE])) {
        if (!sq_attacked(b, OO_ATTACK_MASK[WHITE], BLACK)) {
            add_move(list, from, G1, W_KING, false, false, false, true);
        }
    }
    if ((b.castle_perm & WQCA) && !(b.all_pieces & OOO_MASK[WHITE])) {
        if (!sq_attacked(b, OOO_ATTACK_MASK[WHITE], BLACK)) {
            add_move(list, from, C1, W_KING, false, false, false, true);
        }
    }
}

/**
 * Used to see if any square in a bitmap is under target.
 * smart to use both to see if king is in check, but also check if castling is
 * ok
 */
static int sq_attacked(S_BOARD b, u64 target_bb, int from_side) 
{
    u64 slide_attacks_bb;
    int sq;

    if (from_side == BLACK) {
        while (target_bb) {
            sq = lsb1_index(target_bb);

            if (b.b_pawns & WHITE_PAWN_ATTACKS[sq]) return true;
            if (b.b_knights & KNIGHT_ATTACKS[sq]) return true;
            if (b.b_king & KING_ATTACKS[sq]) return true;

            slide_attacks_bb = b.b_queens | b.b_rooks;
            if (slide_attacks_bb) {
                if (RANK_ATTACKS[sq][(b.all_pieces & RANK_MASK[sq]) >> RANKSHIFT[sq]]
                        & slide_attacks_bb) { 
                    return true; 
                }
                if (FILE_ATTACKS[sq][(b.all_pieces & FILE_MASK[sq]) >> 57]
                        & slide_attacks_bb) { 
                    return true; 
                }
            }

            slide_attacks_bb = b.b_queens | b.b_bishops;
            if (slide_attacks_bb) {
                if (DIAGA8H1_ATTACKS[sq][((b.all_pieces && DIAGA8H1_MASK[sq]) 
                            * DIAGA8H1_MAGIC[sq]) >> 57] & slide_attacks_bb) {
                    return true;
                }
                if (DIAGA1H8_ATTACKS[sq][((b.all_pieces & DIAGA1H8_MASK[sq])
                            * DIAGA1H8_MAGIC[sq]) >> 57] & slide_attacks_bb) {
                    return true;
                }

            }
            target_bb ^= (1LL << sq);
        }

    } else {//SIDE == WHITE
        while (target_bb) {
            sq = lsb1_index(target_bb);

            if (b.w_pawns & BLACK_PAWN_ATTACKS[sq]) return true;
            if (b.w_knights & KNIGHT_ATTACKS[sq]) return true;
            if (b.w_king & KING_ATTACKS[sq]) return true;

            slide_attacks_bb = b.w_queens | b.w_rooks;
            if (slide_attacks_bb) {
                if (RANK_ATTACKS[sq][(b.all_pieces & RANK_MASK[sq]) >> RANKSHIFT[sq]]
                        & slide_attacks_bb) { 
                    return true; 
                }
                if (FILE_ATTACKS[sq][(b.all_pieces & FILE_MASK[sq]) >> 57]
                        & slide_attacks_bb) { 
                    return true; 
                }
            }

            slide_attacks_bb = b.w_queens | b.w_bishops;
            if (slide_attacks_bb) {
                if (DIAGA8H1_ATTACKS[sq][((b.all_pieces && DIAGA8H1_MASK[sq]) 
                            * DIAGA8H1_MAGIC[sq]) >> 57] & slide_attacks_bb) {
                    return true;
                }
                if (DIAGA1H8_ATTACKS[sq][((b.all_pieces & DIAGA1H8_MASK[sq])
                            * DIAGA1H8_MAGIC[sq]) >> 57] & slide_attacks_bb) {
                    return true;
                }
            }
            target_bb ^= (1LL << sq);
        }
    }

    return false;
}

static void add_move(S_MOVELIST *list, int from, int to, int piece, int capture, int promoted, int ep, int castling)
{
    //printf("\nMOVE ADDED! - size: ");
    assert(valid_sq(from));
    assert(valid_sq(to));
    assert(valid_piece(piece));
    assert(valid_piece(capture));
    assert(valid_piece(promoted));
    assert(valid_bool(ep));
    assert(valid_bool(castling));

    list->moves[list->index].move = gen_move(from, to, piece, capture, promoted, ep, castling);
    list->moves[list->index].score = 0;
    list->index++;
    //printf("%d\n", list->index);
}
