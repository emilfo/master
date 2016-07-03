#include <stdio.h>
#include <inttypes.h>
#include "globals.h"
#include "data.h"
#include "hash.h"
#include "hashtable.h"
#include "movegen.h"
#include "io.h"
#include "eval.h"

#define HASH_B(b, val) (b->hash_key ^= val)

const u64 _DIAGA8H1MAGICS[] = {
    0x0,
    0x0,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x0080808080808080,
    0x0040404040404040,
    0x0020202020202020,
    0x0010101010101010,
    0x0008080808080808,
    0x0,
    0x0
};
const u64 _DIAGA1H8MAGICS[] = {
    0x0,
    0x0,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x0101010101010100,
    0x8080808080808000,
    0x4040404040400000,
    0x2020202020000000,
    0x1010101000000000,
    0x0808080000000000,
    0x0,
    0x0
};

const u64 _FILEMAGICS[] = {
    0x8040201008040200,
    0x4020100804020100,
    0x2010080402010080,
    0x1008040201008040,
    0x0804020100804020,
    0x0402010080402010,
    0x0201008040201008,
    0x0100804020100804
};

// Move generator shift for ranks:
const int RANKSHIFT[] = {
    1,  1,  1,  1,  1,  1,  1,  1,
    9,  9,  9,  9,  9,  9,  9,  9,
    17, 17, 17, 17, 17, 17, 17, 17,  
    25, 25, 25, 25, 25, 25, 25, 25,
    33, 33, 33, 33, 33, 33, 33, 33,
    41, 41, 41, 41, 41, 41, 41, 41,
    49, 49, 49, 49, 49, 49, 49, 49,
    57, 57, 57, 57, 57, 57, 57, 57
};

const uint8_t castle_update[64] = {
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    7,  15, 15, 15, 3, 15, 15, 11
};

static void add_piece(S_BOARD *b, int piece, int sq);
static void remove_piece(S_BOARD *b, int piece, int sq);

void init_board(S_BOARD *b) {
    parse_fen(b, START_FEN);
}

int parse_fen(S_BOARD *b, char *fen) {
    int file, rank, piece;
    soft_reset_board(b);

    rank = 8;
    file = 1;
    while((rank >= 1) && *fen) {
        switch (*fen) {
            case 'p': piece = B_PAWN; 
                      break;
            case 'r': piece = B_ROOK;
                      break;
            case 'n': piece = B_KNIGHT;
                      break;
            case 'b': piece = B_BISHOP;
                      break;
            case 'k': piece = B_KING;
                      break;
            case 'q': piece = B_QUEEN;
                      break;
            case 'P': piece = W_PAWN;
                      break;
            case 'R': piece = W_ROOK;
                      break;
            case 'N': piece = W_KNIGHT;
                      break;
            case 'B': piece = W_BISHOP;
                      break;
            case 'K': piece = W_KING; 
                      break;
            case 'Q': piece = W_QUEEN;
                      break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                      piece = EMPTY;
                      file += *fen - '0';
                      break;

            case '/':
            case ' ':
                      rank--;
                      file = 1;
                      fen++;
                      continue;              

            default:
                      printf("FEN error \n");
                      return -1;
        }   

        if (piece != EMPTY) {
            b->piece_bb[piece] |= (1ULL << (FRtoSQ(file, rank))); 
            b->sq[FRtoSQ(file, rank)] = piece;
            file += 1;
        }

        fen++;
    }

    b->all_piece_bb[WHITE] = b->piece_bb[W_PAWN] | b->piece_bb[W_QUEEN] | b->piece_bb[W_ROOK] 
        | b->piece_bb[W_BISHOP] | b->piece_bb[W_KNIGHT] | b->piece_bb[W_KING]; 

    b->all_piece_bb[BLACK] = b->piece_bb[B_PAWN] | b->piece_bb[B_QUEEN] | b->piece_bb[B_ROOK] 
        | b->piece_bb[B_BISHOP] | b->piece_bb[B_KNIGHT] | b->piece_bb[B_KING]; 

    b->all_piece_bb[BOTH] = b->all_piece_bb[WHITE] | b->all_piece_bb[BLACK];

    //debugging added to be certain the FEN board is correctly traversed
    assert(*fen == 'w' || *fen == 'b');

    b->side = (*fen == 'w')? WHITE : BLACK;
    fen += 2;

    while (*fen != ' ') {
        switch (*fen) {
            case 'K': b->castle_perm |= WKCA; break;
            case 'Q': b->castle_perm |= WQCA; break;
            case 'k': b->castle_perm |= BKCA; break;
            case 'q': b->castle_perm |= BQCA; break;
            default: break;
        }
        fen++;
    }
    fen++;

    //check that we didn't somehow screw up castle permissions
    assert((b->castle_perm >= 0b0000) && (b->castle_perm <= 0b1111));

    if (*fen != '-') {
        file = fen[0] - 'a' + 1;
        rank = fen[1] - '1' + 1;

        assert(b->side == WHITE || (rank == 3 && (file >= 1 && file <= 8)));
        assert(b->side == BLACK || (rank == 6 && (file >= 1 && file <= 8)));

        b->ep_sq = FRtoSQ(file, rank);
    }

    //TODO this here?
    b->hash_key = generate_hash(b);

    return 0;
}

void soft_reset_board(S_BOARD *b) 
{
    int i;

    for (i = 0; i < 13; i++) {
        b->piece_bb[i] = 0ULL;
    }

    for (i = 0; i < 3; i++) {
        b->all_piece_bb[i] = 0LL;
    }

    b->side = WHITE;
    b->castle_perm = EMPTY;

    b->ep_sq = EMPTY;
    b->fifty_move_count = 0;

    b->ply = 0;
    b->search_ply = 0;

    for (i = 0; i < 64; i++) {
        b->sq[i] = EMPTY;
    }
}

void hard_reset_board(S_BOARD *b) 
{
    int i, j;
    soft_reset_board(b);

    for (i = 0; i < 13; i++) {
        for (j = 0; j < 64; j++) {
            b->search_history[i][j] = 0;
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < MAX_PLY; j++) {
            b->search_history[i][j] = 0;
        }
    }
}

int make_move(S_BOARD *b, int move)
{
    int from = mv_from(move);
    int to = mv_to(move);
    int piece = mv_piece(move);
    int cap = mv_cap(move);
    int prom = mv_prom(move);

    assert(valid_sq(to));
    assert(valid_sq(from));
    assert(valid_piece(piece));
    assert(valid_piece_or_empty(cap));
    assert(valid_piece_or_empty(prom));

    assert(debug_board(b));

    //save state
    b->prev[b->ply].move = move;
    b->prev[b->ply].castle_perm = b->castle_perm;
    b->prev[b->ply].ep_sq = b->ep_sq;
    b->prev[b->ply].fifty_move_count = b->fifty_move_count;
    b->prev[b->ply].hash_key = b->hash_key;
    b->ply++;

    //update castlepermissions and the corresponding hash_value
    HASH_B(b, castle_key[b->castle_perm]);
    b->castle_perm &= castle_update[from];
    b->castle_perm &= castle_update[to];
    HASH_B(b, castle_key[b->castle_perm]);

    //update en passant sq and the corresponding hash_key
    if (b->ep_sq != EMPTY) {
        HASH_B(b, pce_key[EMPTY][b->ep_sq]);
    }

    b->ep_sq = EMPTY;

    if (piece == W_PAWN) {
        b->fifty_move_count = 0;

        if(ranks[to] == 4 && ranks[from] == 2) {
            b->ep_sq = to - 8;
        }
    } else if (piece == B_PAWN) {
        b->fifty_move_count = 0;

        if(ranks[to] == 5 && ranks[from] == 7) {
            b->ep_sq = to + 8;
        }
    }

    if (mv_castle(move)) {
        if (b->side) { //BLACK
            if(to == C8) {
                remove_piece(b, B_ROOK, A8);
                add_piece(b, B_ROOK, D8);
            } else {
                remove_piece(b, B_ROOK, H8);
                add_piece(b, B_ROOK, F8);
            }
        } else { //WHITE
            if(to == C1) {
                remove_piece(b, W_ROOK, A1);
                add_piece(b, W_ROOK, D1);
            } else {
                remove_piece(b, W_ROOK, H1);
                add_piece(b, W_ROOK, F1);
            }
        }

    } else if (mv_ep(move)) {
        if (b->side) { //BLACK
            remove_piece(b, W_PAWN, to + 8);
        } else { //WHITE
            remove_piece(b, B_PAWN, to - 8);
        }
    } else if (cap) { 
        b->fifty_move_count = 0;
        remove_piece(b, cap, to); 
    }

    //doing the actual move
    remove_piece(b, piece, from);

    if (prom) { 
        piece =  prom;
    }

    add_piece(b, piece, to);

    b->side = 1 - b->side;

    if (b->side == BLACK) {
        if (sq_attacked(b, b->piece_bb[W_KING], BLACK)) {
            unmake_move(b);
            return 0;
        }
    } else { //WHITE
        if (sq_attacked(b, b->piece_bb[B_KING], WHITE)) {
            unmake_move(b);
            return 0;
        }
    }

    assert(debug_board(b));

    return 1;
}

void unmake_move(S_BOARD *b)
{
    b->ply--;

    int move = b->prev[b->ply].move;
    int from = mv_from(move);
    int to = mv_to(move);
    int piece = mv_piece(move);
    int cap = mv_cap(move);
    int prom = mv_prom(move);

    assert(debug_board(b));

    if (prom) { 
        remove_piece(b, prom, to);
    } else {
        remove_piece(b, piece, to);
    }

    add_piece(b, piece, from);

    if (mv_castle(move)) {
        if (b->side) { //BLACK (unmaking whites move)
            if(to == C1) {
                remove_piece(b, W_ROOK, D1);
                add_piece(b, W_ROOK, A1);
            } else {
                remove_piece(b, W_ROOK, F1);
                add_piece(b, W_ROOK, H1);
            }
        } else { //WHITE
            if(to == C8) {
                remove_piece(b, B_ROOK, D8);
                add_piece(b, B_ROOK, A8);
            } else {
                remove_piece(b, B_ROOK, F8);
                add_piece(b, B_ROOK, H8);
            }
        }

    } 

    else if (mv_ep(move)) {
        if (b->side) { //BLACK
            add_piece(b, B_PAWN, to - 8);
        } else { //WHITE
            add_piece(b, W_PAWN, to + 8);
        }
    } 

    else if (cap) { 
        add_piece(b, cap, to); 
    }

    //restore state
    b->castle_perm = b->prev[b->ply].castle_perm;
    b->ep_sq = b->prev[b->ply].ep_sq;
    b->fifty_move_count = b->prev[b->ply].fifty_move_count;
    b->hash_key = b->prev[b->ply].hash_key;
    b->side = 1 - b->side;

    assert(debug_board(b));
}

static void add_piece(S_BOARD *b, int piece, int sq) {
    assert(valid_sq(sq));
    assert(valid_piece(piece));
    assert(b->sq[sq] == EMPTY);

    b->sq[sq] = piece;
    HASH_B(b, pce_key[piece][sq]);
    b->piece_bb[piece] |= (1LL << sq);
    b->all_piece_bb[PIECE_COLOR[piece]] |= (1LL << sq);
    b->all_piece_bb[BOTH] |= (1LL << sq);
}

static void remove_piece(S_BOARD *b, int piece, int sq) {
    assert(valid_sq(sq));
    assert(valid_piece(piece));
    assert(b->sq[sq] == piece);

    b->sq[sq] = EMPTY;
    HASH_B(b, pce_key[piece][sq]);

    b->piece_bb[piece] ^= (1LL << sq);
    b->all_piece_bb[PIECE_COLOR[piece]] ^= (1LL << sq);
    b->all_piece_bb[BOTH] ^= (1LL << sq);
}

int make_move_if_exist(S_BOARD *b, int move)
{
    int i;
    S_MOVELIST l[1];

    generate_all_moves(b, l);

    for (i = 0; i < l->index; i++) {
        if (l->moves[i].move == move && make_move(b, move)) {
            return true;
        }
    }
    return false;
}

void flip_board(S_BOARD *b)
{
    int i;
    u64 swp_bb;

    //Flipping piece bitboards
    for (i = 1; i < 7; i++) {
        swp_bb = flipVertical(b->piece_bb[i]);
        b->piece_bb[i] = flipVertical(b->piece_bb[i+6]);
        b->piece_bb[i+6] = swp_bb;
    }
    //Flipping white and black bitboards
    swp_bb = flipVertical(b->all_piece_bb[WHITE]);
    b->all_piece_bb[WHITE] = flipVertical(b->all_piece_bb[BLACK]);
    b->all_piece_bb[BLACK] = swp_bb;

    //Flipping all pieces board
    b->all_piece_bb[BOTH] = flipVertical(b->all_piece_bb[BOTH]);

    //Flipping side to move
    b->side = 1 - b->side;

    //castleperm
    uint8_t new_castle_perm = 0;
    if (b->castle_perm & BKCA) new_castle_perm |= WKCA;
    if (b->castle_perm & BQCA) new_castle_perm |= WQCA;
    if (b->castle_perm & WKCA) new_castle_perm |= BKCA;
    if (b->castle_perm & WQCA) new_castle_perm |= BQCA;
    b->castle_perm = new_castle_perm;

    //en passant square
    if (b->ep_sq) b->ep_sq = mirror[b->ep_sq];

    //the piece-board
    uint8_t new_sq[64];
    for (i = 0; i < 64; i++) {
        new_sq[i] = b->sq[mirror[i]];
    }
    for (i = 0; i < 64; i++) {
        b->sq[i] = new_sq[i];
    }

    b->hash_key = generate_hash(b);

    debug_board(b);
}

void print_board(const S_BOARD *b) {
    int file, rank;

    for (rank = 8; rank >= 1; rank--) {
        printf("  +---+---+---+---+---+---+---+---+\n%d |", rank);
        for (file = 1; file <= 8; file++) {
            printf("%2c |", PIECE_NAME[b->sq[FRtoSQ(file, rank)]]);
        }
        printf("\n");
    }

    printf("  +---+---+---+---+---+---+---+---+\n");
    printf("    a   b   c   d   e   f   g   h  \n");

    printf("side:%c, castle_perm:%d, ep:%i, fifty_move:%i\n",
            (b->side)?'B':'W', b->castle_perm, b->ep_sq,
            b->fifty_move_count);
}

/* This is a help mehtod only used in debugging */
int debug_board(S_BOARD *b) {
    int i, j;

    for (i = A1; i <= H8; i++) {
        if (b->sq[i] == EMPTY) {
            for (j = 0; j < 13; j++) {
                assert(!((1ULL << i) & b->piece_bb[j]));
            }
        } else {
            assert((1ULL << i) & b->piece_bb[b->sq[i]]);
            assert((1ULL << i) & b->all_piece_bb[PIECE_COLOR[b->sq[i]]]);
            assert((1ULL << i) & b->all_piece_bb[BOTH]);
        }
    }

    if(b->ep_sq != EMPTY) {
        assert((ranks[b->ep_sq] == 6 && b->side == WHITE) ||
                (ranks[b->ep_sq] == 3 && b->side == BLACK));
    }

    return true;
}

void print_bitboard(BIT_BOARD *bboard) {
    int i;

    for (i = 7; i >= 0; i--) {
        print_bitboard_rank(bboard->rank[i]);
    }
}

void print_bitboard_rank(uint8_t rank) {
    int i;
    for (i = 0; i < 8; i++) {
        printf("%d ", ((1 << i) & rank)? 1 : 0);
    }

    printf("\n");
}
