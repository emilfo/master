#include <stdio.h>
#include "board.h"
#include "globals.h"
#include "eval.h"

const int mirror[64] = {
    56  ,   57  ,   58  ,   59  ,   60  ,   61  ,   62  ,   63  ,
    48  ,   49  ,   50  ,   51  ,   52  ,   53  ,   54  ,   55  ,
    40  ,   41  ,   42  ,   43  ,   44  ,   45  ,   46  ,   47  ,
    32  ,   33  ,   34  ,   35  ,   36  ,   37  ,   38  ,   39  ,
    24  ,   25  ,   26  ,   27  ,   28  ,   29  ,   30  ,   31  ,
    16  ,   17  ,   18  ,   19  ,   20  ,   21  ,   22  ,   23  ,
    8   ,   9   ,   10  ,   11  ,   12  ,   13  ,   14  ,   15  ,
    0   ,   1   ,   2   ,   3   ,   4   ,   5   ,   6   ,   7   
};

const int PAWN_SQ_VAL[64] = {
    0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,
    10  ,   10  ,   0   ,   -10 ,   -10 ,   0   ,   10  ,   10  ,
    5   ,   0   ,   0   ,   5   ,   5   ,   0   ,   0   ,   5   ,
    0   ,   0   ,   10  ,   20  ,   20  ,   10  ,   0   ,   0   ,
    5   ,   5   ,   5   ,   10  ,   10  ,   5   ,   5   ,   5   ,
    10  ,   10  ,   10  ,   20  ,   20  ,   10  ,   10  ,   10  ,
    20  ,   20  ,   20  ,   30  ,   30  ,   20  ,   20  ,   20  ,
    0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   
};

const int KNIGHT_SQ_VAL[64] = {
    0   ,   -10 ,   5   ,   0   ,   0   ,   5   ,   -10 ,   0   ,
    0   ,   0   ,   0   ,   5   ,   5   ,   0   ,   0   ,   0   ,
    0   ,   0   ,   10  ,   10  ,   10  ,   10  ,   0   ,   0   ,
    0   ,   5   ,   10  ,   20  ,   20  ,   10  ,   5   ,   0   ,
    5   ,   10  ,   15  ,   20  ,   20  ,   15  ,   10  ,   5   ,
    10  ,   15  ,   15  ,   25  ,   25  ,   15  ,   15  ,   10  ,
    0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
    0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0       
};

const int BISHOP_SQ_VAL[64] = {
    -5  ,   0   ,   -10 ,   0   ,   0   ,   -10 ,   0   ,   -5  ,
    0   ,   20  ,   0   ,   15  ,   15  ,   0   ,   20  ,   0   ,
    0   ,   0   ,   15  ,   10  ,   10  ,   15  ,   5   ,   0   ,
    0   ,   10  ,   0   ,   15  ,   15  ,   10  ,   5   ,   5   ,
    0   ,   10  ,   0   ,   15  ,   15  ,   10  ,   10  ,   0   ,
    0   ,   0   ,   15  ,   5   ,   5   ,   15  ,   0   ,   0   ,
    0   ,   15  ,   0   ,   10  ,   10  ,   0   ,   15  ,   0   ,
    -5  ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   -5  
};

const int ROOK_SQ_VAL[64] = {
    0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
    0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
    0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
    0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
    0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
    0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
    25  ,   25  ,   25  ,   25  ,   25  ,   25  ,   25  ,   25  ,
    0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0       
};

const int QUEEN_SQ_VAL[64] = {
    0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,
    0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,
    0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,
    0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,
    0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,
    0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,
    0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,
    0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0       
};

const int KING_SQ_VAL_END[64] = { 
    -50 ,   -10 ,   0   ,   0   ,   0   ,   0   ,   -10 ,   -50 ,
    -10,    0   ,   10  ,   10  ,   10  ,   10  ,   0   ,   -10 ,
    0   ,   10  ,   15  ,   15  ,   15  ,   15  ,   10  ,   0   ,
    0   ,   10  ,   15  ,   20  ,   20  ,   15  ,   10  ,   0   ,
    0   ,   10  ,   15  ,   20  ,   20  ,   15  ,   10  ,   0   ,
    0   ,   10  ,   15  ,   15  ,   15  ,   15  ,   10  ,   0   ,
    -10,    0   ,   10  ,   10  ,   10  ,   10  ,   0   ,   -10 ,
    -50 ,   -10 ,   0   ,   0   ,   0   ,   0   ,   -10 ,   -50 
};

const int KING_SQ_VAL[64] = { 
    0   ,   10  ,   10  ,   -10 ,   -10 ,   10  ,   20  ,   5   ,
    0   ,   0   ,   5   ,   0   ,   0   ,   0   ,   5   ,   0   ,
    -10 ,   -10 ,   -10 ,   -10 ,   -10 ,   -10 ,   -10 ,   -10 ,
    -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70 ,
    -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70 ,
    -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70 ,
    -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70 ,
    -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70 ,   -70     
};

/* Evaluates position from the one to moves side of view */
int eval_posistion(const S_BOARD *b) 
{
    u64 cur_piece_bb;
    int sq;
    int score = 0;

    cur_piece_bb = b->piece_bb[B_PAWN];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score -= PIECE_VAL[B_PAWN] + PAWN_SQ_VAL[mirror[sq]];

        cur_piece_bb ^= (1LL << sq);
    }

    cur_piece_bb = b->piece_bb[W_PAWN];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score += PIECE_VAL[W_PAWN] + PAWN_SQ_VAL[sq];

        cur_piece_bb ^= (1LL << sq);
    }

    cur_piece_bb = b->piece_bb[B_KNIGHT];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score -= PIECE_VAL[B_KNIGHT] + KNIGHT_SQ_VAL[mirror[sq]];

        cur_piece_bb ^= (1LL << sq);
    }

    cur_piece_bb = b->piece_bb[W_KNIGHT];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score += PIECE_VAL[W_KNIGHT] + KNIGHT_SQ_VAL[sq];

        cur_piece_bb ^= (1LL << sq);
    }

    cur_piece_bb = b->piece_bb[B_BISHOP];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score -= PIECE_VAL[B_BISHOP] + BISHOP_SQ_VAL[mirror[sq]];

        cur_piece_bb ^= (1LL << sq);
    }

    cur_piece_bb = b->piece_bb[W_BISHOP];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score += PIECE_VAL[W_BISHOP] + BISHOP_SQ_VAL[sq];

        cur_piece_bb ^= (1LL << sq);
    }

    cur_piece_bb = b->piece_bb[B_ROOK];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score -= PIECE_VAL[B_ROOK] + ROOK_SQ_VAL[mirror[sq]];

        cur_piece_bb ^= (1LL << sq);
    }

    cur_piece_bb = b->piece_bb[W_ROOK];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score += PIECE_VAL[W_ROOK] + ROOK_SQ_VAL[sq];

        cur_piece_bb ^= (1LL << sq);
    }

    cur_piece_bb = b->piece_bb[B_QUEEN];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score -= PIECE_VAL[B_QUEEN] + QUEEN_SQ_VAL[mirror[sq]];

        cur_piece_bb ^= (1LL << sq);
    }

    cur_piece_bb = b->piece_bb[W_QUEEN];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score += PIECE_VAL[W_QUEEN] + QUEEN_SQ_VAL[sq];

        cur_piece_bb ^= (1LL << sq);
    }

    cur_piece_bb = b->piece_bb[B_KING];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score -= PIECE_VAL[B_KING] + KING_SQ_VAL[mirror[sq]];

        cur_piece_bb ^= (1LL << sq);
    }

    cur_piece_bb = b->piece_bb[W_KING];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score += PIECE_VAL[W_KING] + KING_SQ_VAL[sq];

        cur_piece_bb ^= (1LL << sq);
    }

    if (b->side) { //BLACK
        return -score;
    } else { //WHITE
        return score;
    }
}

void test() 
{
    int i;
    for (i = 0; i < 64; i++) {
        printf("\n%d:\n", i);
        print_bitboard((BIT_BOARD *) &BLACK_PASSED_MASK[i]);
        printf("\n\n");
        u64 flip = flipVertical(BLACK_PASSED_MASK[i]);
        print_bitboard((BIT_BOARD *) &flip);
        printf("\n\n");
    }
}
