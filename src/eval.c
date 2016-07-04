#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "globals.h"
#include "eval.h"

const int PAWN_ISOLATED = -20;
const int WHITE_PAWN_PASSED[8] = {0, 5, 10, 20, 40, 80, 160, 0};
const int BLACK_PAWN_PASSED[8] = {0, 160, 80, 40, 20, 10, 5, 0};
const int ROOK_OPEN = 50;
const int ROOK_SEMI_OPEN = 20;

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

//based off wikipedia article (assumes alot)
static int material_draw(const S_BOARD *b) {
    if (b->piece_bb[B_PAWN] || b->piece_bb[W_PAWN] || b->piece_bb[B_QUEEN] || b->piece_bb[W_QUEEN]) {
        return false;
    }

    int black_bishops = bit_count(b->piece_bb[B_BISHOP]);
    int white_bishops = bit_count(b->piece_bb[W_BISHOP]);
    int black_knights = bit_count(b->piece_bb[B_KNIGHT]);
    int white_knights = bit_count(b->piece_bb[W_KNIGHT]);
    int black_rooks = bit_count(b->piece_bb[B_ROOK]);
    int white_rooks = bit_count(b->piece_bb[W_ROOK]);


    if (white_rooks == 0 && black_rooks == 0) {
        if (black_bishops + black_knights + white_bishops + white_knights == 2) {
            if (black_bishops == 2 || white_bishops == 2) return false;
            if (black_bishops == 1 || black_knights == 1) return false;
            if (white_bishops == 1 || white_knights == 1) return false;
            return true;
        } else if (black_bishops + black_knights + white_bishops + white_knights == 3) {
            if (black_bishops + black_knights == 3) return false;
            if (white_bishops + white_knights == 3) return false;
            if (black_bishops == 2 && white_knights == 1) return false;
            if (white_bishops == 2 && black_knights == 1) return false;
            return true;
        } else if (white_knights == 0 && black_knights == 0) {
            if (abs( black_bishops - white_bishops) <= 1) return true;
            return false;
        } else if (white_bishops  == 0 && black_bishops == 0) {
            if (white_knights < 3 && black_knights < 3) return true;
            return false;
        }
        return false;
    }

    if (black_rooks == 0 && white_rooks == 1) {
        if (white_bishops == 0 && white_knights == 0) {
            if (black_bishops + black_knights == 2) return true;
            return false;
        }
        if (white_bishops == 1 && white_knights == 0) {
            if (black_bishops + black_knights == 2 && black_knights != 2) return true;
            return false;
        }
        if (white_bishops == 0 && white_knights == 1) {
            if (black_bishops == 0 && white_knights == 2) return true;
            return false;
        }
    } else if (black_rooks == 1 && white_rooks == 0) {
        if (black_bishops == 0 && black_knights == 0) {
            if (white_bishops + white_knights == 2) return true;
            return false;
        }
        if (black_bishops == 1 && black_knights == 0) {
            if (white_bishops + white_knights == 2 && white_knights != 2) return true;
            return false;
        }
        if (black_bishops == 0 && black_knights == 1) {
            if (white_bishops == 0 && black_knights == 2) return true;
            return false;
        }
    } else if (black_rooks == 1 && white_rooks == 1){
        if (black_bishops + black_knights < 2 && white_knights + white_bishops < 2) return true;
    } else if (black_rooks == 2 && white_rooks == 0){
        if (white_bishops + white_knights == 3) return true;
    } else if (black_rooks == 0 && white_rooks == 2){
        if (black_bishops + black_knights == 3) return true;
    }
    return false;

}

/* Evaluates position from the one to moves side of view */
int eval_posistion(const S_BOARD *b) 
{
    u64 cur_piece_bb;
    int sq;
    int score = 0;

    if (material_draw(b)) {
        printf("\n\nGOT HERE\n\n");
        return 0;
    }

    cur_piece_bb = b->piece_bb[B_PAWN];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score -= PIECE_VAL[B_PAWN] + PAWN_SQ_VAL[mirror[sq]];

        if (ISOLATED_PAWN_MASK[sq] & b->piece_bb[B_PAWN]) {
            score -= PAWN_ISOLATED;
        }

        if (BLACK_PAWN_PASSED[sq] & b->piece_bb[W_PAWN]) {
            score -= BLACK_PASSED_MASK[files[sq] - 1];
        }

        cur_piece_bb ^= (1LL << sq);
    }

    cur_piece_bb = b->piece_bb[W_PAWN];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score += PIECE_VAL[W_PAWN] + PAWN_SQ_VAL[sq];

        if (ISOLATED_PAWN_MASK[sq] & b->piece_bb[W_PAWN]) {
            score += PAWN_ISOLATED;
        }

        if (WHITE_PAWN_PASSED[sq] & b->piece_bb[B_PAWN]) {
            score += WHITE_PASSED_MASK[files[sq] - 1];
        }

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

        if (!(EVAL_FILE_MASK[files[sq]-1] & b->piece_bb[B_PAWN])) {
            if (!(EVAL_FILE_MASK[files[sq]-1] & b->piece_bb[W_PAWN])) {
                score -= ROOK_OPEN;
            } else {
                score -= ROOK_SEMI_OPEN;
            }
        }

        cur_piece_bb ^= (1LL << sq);
    }

    cur_piece_bb = b->piece_bb[W_ROOK];
    while(cur_piece_bb) {
        sq = lsb1_index(cur_piece_bb);

        score += PIECE_VAL[W_ROOK] + ROOK_SQ_VAL[sq];

        if (!(EVAL_FILE_MASK[files[sq]-1] & b->piece_bb[W_PAWN])) {
            if (!(EVAL_FILE_MASK[files[sq]-1] & b->piece_bb[B_PAWN])) {
                score += ROOK_OPEN;
            } else {
                score += ROOK_SEMI_OPEN;
            }
        }

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
