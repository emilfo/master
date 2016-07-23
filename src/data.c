#include <stdio.h>

//#include "globals.h"
#include "defs.h"
#include "bitops.h"
#include "data.h"
#include "eval.h"
#include "board.h"

#define B_INDEX(file, rank) ((rank-1)*8 + (file-1))
#define MIN(a, b) ((a < b)? a : b)

static int valid_file_rank(int file, int rank) {
    return ((file >= 1) && (file <= 8) && (rank >=1) && (rank <= 8));
}

static void init_ms1btable() {
    int i;
    for (i = 0; i < 256; i++) {
        MS1BTABLE[i] = (
                (i >= 128)? 7 :
                (i >= 64) ? 6 :
                (i >= 32) ? 5 :
                (i >= 16) ? 4 :
                (i >= 8)  ? 3 :
                (i >= 4)  ? 2 :
                (i >= 2)  ? 1 : 0);
    }
}

static void init_bitboard_mask() {
    int sq, rank, file, diaga1h8, diaga8h1;

    printf("reset masks and magics\n");
    for (sq = 0; sq < 64; sq++) {
        RANK_MASK[sq] = 0x0;
        FILE_MASK[sq] = 0x0;
        DIAGA1H8_MASK[sq] = 0x0;
        DIAGA8H1_MASK[sq] = 0x0;
        FILE_MAGIC[sq] = 0x0;
        DIAGA1H8_MAGIC[sq] = 0x0;
        DIAGA8H1_MAGIC[sq] = 0x0;
    }


    printf("rank file and diag masks\n");
    for (rank = 1; rank <= 8; rank++) {
        for (file = 1; file <= 8; file++) {
            int index = B_INDEX(file, rank);

            //6-bit rank and file mask used for movegen
            RANK_MASK[index] = (1LL << B_INDEX(2, rank)) | (1LL << B_INDEX(3, rank))
                | (1LL << B_INDEX(4, rank)) | (1LL << B_INDEX(5, rank)) 
                | (1LL << B_INDEX(6, rank)) | (1LL << B_INDEX(7, rank));

            FILE_MASK[index] = (1LL << B_INDEX(file, 2)) | (1LL << B_INDEX(file,3))
                | (1LL << B_INDEX(file, 4)) | (1LL << B_INDEX(file, 5)) 
                | (1LL << B_INDEX(file, 6)) | (1LL << B_INDEX(file, 7));

            diaga8h1 = file + rank;
            DIAGA8H1_MAGIC[index] = _DIAGA8H1MAGICS[diaga8h1 - 2];

            DIAGA8H1_MASK[index] = 0x0;
            if (diaga8h1 < 10) {
                for (sq = 2; sq < diaga8h1-1; sq++) {
                    DIAGA8H1_MASK[index] |= (1LL << B_INDEX(sq, diaga8h1-sq));
                }
            } else {
                for (sq = 2; sq < 17 - diaga8h1; sq++) {
                    DIAGA8H1_MASK[index] |= (1LL << B_INDEX(diaga8h1 +sq-9, 9-sq));
                }
            }

            diaga1h8 = file - rank;
            DIAGA1H8_MAGIC[index] = _DIAGA1H8MAGICS[diaga1h8 + 7];

            DIAGA1H8_MASK[index] = 0x0;
            if (diaga1h8 > -1) {
                for (sq = 2; sq < 8-diaga1h8; sq++) {
                    DIAGA1H8_MASK[index] |= (1LL << B_INDEX(diaga1h8+sq, sq));
                }
            } else {
                for (sq = 2; sq < 8 + diaga1h8; sq++) {
                    DIAGA1H8_MASK[index] |= (1LL << B_INDEX(sq, sq-diaga1h8));
                }
            }

            FILE_MAGIC[index] = _FILEMAGICS[file-1];
        }
    }

    printf("gen sliding attacks\n");
    unsigned char state_6bit, state_8bit, attack_8bit;
    int slide;

    //    unsigned char CHARBITSET[64];

    for (sq = 0; sq <= 7; sq++) {

        //loop of occupancy states
        //state_6bit represents the 64 possible occupancy states of a rank
        //except the end-bits (because edges stop either way)
        for (state_6bit = 0; state_6bit < 64; state_6bit ++) {

            state_8bit = state_6bit << 1;
            attack_8bit = 0;

            if (sq < 7) {
                attack_8bit |= (1 << (sq+1));
            }

            slide = sq + 2;
            while (slide <= 7) {
                if((~state_8bit & (1 << (slide-1)))) {
                    attack_8bit |= (1 << slide);
                } else {
                    break;
                }
                slide++;
            }

            if (sq > 0) {
                attack_8bit |= (1 << (sq-1));
            }

            slide = sq - 2;
            while (slide >= 0) {
                if((~state_8bit) & (1 << (slide + 1))) {
                    attack_8bit |= (1 << slide);
                } else {
                    break;
                }
                slide--;
            }

            GEN_SLIDING_ATTACKS[sq][state_6bit] = attack_8bit;
        }
    }
}

static void init_attack_bitmaps() {
    int sq, state, file, rank, a_file, a_rank;
    unsigned char state_6bit;

    for (sq = 0; sq < 64; sq++) {
        KNIGHT_ATTACKS[sq] = 0LL;
        KING_ATTACKS[sq] = 0LL;
        WHITE_PAWN_ATTACKS[sq] = 0LL;
        WHITE_PAWN_MOVES[sq] = 0LL;
        WHITE_PAWN_DOUBLE_MOVES[sq] = 0LL;
        BLACK_PAWN_ATTACKS[sq] = 0LL;
        BLACK_PAWN_MOVES[sq] = 0LL;
        BLACK_PAWN_DOUBLE_MOVES[sq] = 0LL;   

        for (state = 0; state < 64; state++)
        {
            RANK_ATTACKS[sq][state] = 0LL;
            FILE_ATTACKS[sq][state] = 0LL;
            DIAGA8H1_ATTACKS[sq][state] = 0LL;
            DIAGA1H8_ATTACKS[sq][state] = 0LL;
        }
    }

    //White pawn moves
    for (sq = 0; sq < 64; sq++) {
        file = files[sq];
        rank = ranks[sq];

        a_file = file;
        a_rank = rank + 1;
        if (valid_file_rank(a_file, a_rank)) {
            WHITE_PAWN_MOVES[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        if (rank == 2) {
            a_file = file;
            a_rank = rank + 2;
            if (valid_file_rank(a_file, a_rank)) {
                WHITE_PAWN_DOUBLE_MOVES[sq] |= (1LL << B_INDEX(a_file, a_rank));
            }
        }
    }

    //White pawn attacks
    for (sq = 0; sq < 64; sq++) {
        file = files[sq];
        rank = ranks[sq];

        a_file = file - 1;
        a_rank = rank + 1;
        if (valid_file_rank(a_file, a_rank)) {
            WHITE_PAWN_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file + 1;
        a_rank = rank + 1;
        if (valid_file_rank(a_file, a_rank)) {
            WHITE_PAWN_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }
    }

    //Black pawn moves
    for (sq = 0; sq < 64; sq++) {
        file = files[sq];
        rank = ranks[sq];

        a_file = file;
        a_rank = rank - 1;
        if (valid_file_rank(a_file, a_rank)) {
            BLACK_PAWN_MOVES[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        if (rank == 7) {
            a_file = file;
            a_rank = rank - 2;
            if (valid_file_rank(a_file, a_rank)) {
                BLACK_PAWN_DOUBLE_MOVES[sq] |= (1LL << B_INDEX(a_file, a_rank));
            }
        }
    }

    //Black pawn attacks
    for (sq = 0; sq < 64; sq++) {
        file = files[sq];
        rank = ranks[sq];

        a_file = file - 1;
        a_rank = rank - 1;
        if (valid_file_rank(a_file, a_rank)) {
            BLACK_PAWN_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file + 1;
        a_rank = rank - 1;
        if (valid_file_rank(a_file, a_rank)) {
            BLACK_PAWN_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }
    }

    //Knight attacks
    for (sq = 0; sq < 64; sq++) {
        file = files[sq];
        rank = ranks[sq];

        a_file = file + 2;
        a_rank = rank + 1;
        if(valid_file_rank(a_file, a_rank)) {
            KNIGHT_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file + 2;
        a_rank = rank - 1;
        if(valid_file_rank(a_file, a_rank)) {
            KNIGHT_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file - 2;
        a_rank = rank + 1;
        if(valid_file_rank(a_file, a_rank)) {
            KNIGHT_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file - 2;
        a_rank = rank - 1;
        if(valid_file_rank(a_file, a_rank)) {
            KNIGHT_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file + 1;
        a_rank = rank + 2;
        if(valid_file_rank(a_file, a_rank)) {
            KNIGHT_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file + 1;
        a_rank = rank - 2;
        if(valid_file_rank(a_file, a_rank)) {
            KNIGHT_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file - 1;
        a_rank = rank + 2;
        if(valid_file_rank(a_file, a_rank)) {
            KNIGHT_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file - 1;
        a_rank = rank - 2;
        if(valid_file_rank(a_file, a_rank)) {
            KNIGHT_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }
    }

    //King attacks
    for (sq = 0; sq < 64; sq++) {
        file = files[sq];
        rank = ranks[sq];

        a_file = file;
        a_rank = rank + 1;
        if(valid_file_rank(a_file, a_rank)) {
            KING_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file;
        a_rank = rank - 1;
        if(valid_file_rank(a_file, a_rank)) {
            KING_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file + 1;
        a_rank = rank;
        if(valid_file_rank(a_file, a_rank)) {
            KING_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file - 1;
        a_rank = rank;
        if(valid_file_rank(a_file, a_rank)) {
            KING_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file + 1;
        a_rank = rank + 1;
        if(valid_file_rank(a_file, a_rank)) {
            KING_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file + 1;
        a_rank = rank - 1;
        if(valid_file_rank(a_file, a_rank)) {
            KING_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file - 1;
        a_rank = rank + 1;
        if(valid_file_rank(a_file, a_rank)) {
            KING_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }

        a_file = file - 1;
        a_rank = rank - 1;
        if(valid_file_rank(a_file, a_rank)) {
            KING_ATTACKS[sq] |= (1LL << B_INDEX(a_file, a_rank));
        }
    }

    //RANK attacks (for rooks and queens)
    for (sq = 0; sq < 64; sq++) {
        for (state_6bit = 0; state_6bit < 64; state_6bit++) {
            RANK_ATTACKS[sq][state_6bit] = 0LL;
            RANK_ATTACKS[sq][state_6bit] |= 
                (u64) GEN_SLIDING_ATTACKS[files[sq]-1][state_6bit] << (RANKSHIFT[sq]-1);
        }
    }

    int attackbit, pos, diaga1h8, diaga8h1;
    for (sq = 0; sq < 64; sq++) {
        for (state_6bit = 0; state_6bit < 64; state_6bit++) {
            FILE_ATTACKS[sq][state_6bit] = 0LL;

            for(attackbit = 0; attackbit < 8; attackbit++) {
                if(GEN_SLIDING_ATTACKS[8-ranks[sq]][state_6bit] & (1 << attackbit)) {
                    file = files[sq];
                    rank = 8 - attackbit;
                    FILE_ATTACKS[sq][state_6bit] |= (1LL << B_INDEX(file, rank));
                }
            }
        }
    }


    for (sq = 0; sq < 64; sq++) {
        for (state_6bit = 0; state_6bit < 64; state_6bit++) {

            DIAGA8H1_ATTACKS[sq][state_6bit] = 0LL;
            for(attackbit = 0; attackbit < 8; attackbit++) {

                pos = MIN(8-ranks[sq], files[sq]-1);
                if (GEN_SLIDING_ATTACKS[pos][state_6bit] & (1 << attackbit)) {

                    diaga8h1 = files[sq] + ranks[sq];
                    if (diaga8h1 < 10) {
                        file = attackbit + 1;
                        rank = diaga8h1 - file;
                    } else {
                        rank = 8 - attackbit;
                        file = diaga8h1 - rank;
                    }

                    if (valid_file_rank(file, rank)) {
                        DIAGA8H1_ATTACKS[sq][state_6bit] |= (1LL << B_INDEX (file, rank));
                    }
                }
            }
        }
    }


    for (sq = 0; sq < 64; sq++) {
        for (state_6bit = 0; state_6bit < 64; state_6bit++) {

            DIAGA1H8_ATTACKS[sq][state_6bit] = 0LL;
            for(attackbit = 0; attackbit < 8; attackbit++) {

                pos = MIN(ranks[sq]-1, files[sq]-1);
                if (GEN_SLIDING_ATTACKS[pos][state_6bit] & (1 << attackbit)) {

                    diaga1h8 = files[sq] - ranks[sq];
                    if (diaga1h8 < 0) {
                        file = attackbit + 1;
                        rank = file - diaga1h8;
                    } else {
                        rank = attackbit + 1;
                        file = diaga1h8 + rank;
                    }

                    if(valid_file_rank(file, rank)) {
                        DIAGA1H8_ATTACKS[sq][state_6bit] |= (1LL << B_INDEX(file, rank));
                    }
                }
            }
        }
    }

    //castling masks
    OO_MASK[WHITE] = (1LL << F1) | (1LL << G1);
    OO_MASK[BLACK] = (1LL << F8) | (1LL << G8);
    OOO_MASK[WHITE] = (1LL << B1) | (1LL << C1) | (1LL << D1);
    OOO_MASK[BLACK] = (1LL << B8) | (1LL << C8) | (1LL << D8);

    /* To check if king is in check/will be in check in the move, we don't
     * bother checking where king ends up, as this will be checked when
     * generating the move, either way
     */
    OO_ATTACK_MASK[WHITE] = (1LL << E1) | (1LL << F1);
    OO_ATTACK_MASK[BLACK] = (1LL << E8) | (1LL << F8);
    OOO_ATTACK_MASK[WHITE] = (1LL << E1) | (1LL << D1);
    OOO_ATTACK_MASK[BLACK] = (1LL << E8) | (1LL << D8);
}

static void init_evaluate_mask()
{
    int i;
    int file_index, rank_index;

    for (i = 0; i < 8; i++) {
        EVAL_RANK_MASK[i] = 0LL;
        EVAL_FILE_MASK[i] = 0LL;
    }

    for (i = 0; i < 64; i++) {
        EVAL_RANK_MASK[ranks[i]-1] |= (1LL << i);
        EVAL_FILE_MASK[files[i]-1] |= (1LL << i);
    }

    for (i = 0; i < 64; i++) {
        ISOLATED_PAWN_MASK[i] = 0LL;
        WHITE_PASSED_MASK[i] = 0LL;
        BLACK_PASSED_MASK[i] = 0LL;
    }

    for (i = 0; i < 64; i++) {
        file_index = files[i] - 2;
        if (file_index >= 0) {
            ISOLATED_PAWN_MASK[i] |= EVAL_FILE_MASK[file_index];
        }
        file_index = files[i];
        if (file_index < 8) {
            ISOLATED_PAWN_MASK[i] |= EVAL_FILE_MASK[file_index];
        }

        for (file_index = files[i]-2; file_index <= files[i]; file_index++) {
            for (rank_index = 7; rank_index > ranks[i]-1; rank_index--) {
                if (file_index < 8 && file_index >=0) {
                    WHITE_PASSED_MASK[i] |= (EVAL_FILE_MASK[file_index] & EVAL_RANK_MASK[rank_index]);
                }
            }

            for (rank_index = 0; rank_index < ranks[i]-1; rank_index++) {
                if (file_index < 8 && file_index >=0) {
                    BLACK_PASSED_MASK[i] |= (EVAL_FILE_MASK[file_index] & EVAL_RANK_MASK[rank_index]);
                }
            }
        }

    }

    for (i = 0; i < 64; i++) {
    }
}

void init_data() 
{
    printf("ms1table\n");
    init_ms1btable();
    printf("mask\n");
    init_bitboard_mask();
    printf("attack bitmaps\n");
    init_attack_bitmaps();
    printf("evaluate masks\n");
    init_evaluate_mask();
}
