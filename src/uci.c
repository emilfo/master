#include <stdint.h>
#include <string.h>

#include "uci.h"
#include "globals.h"

void move_string(uint32_t move, char *mv_str)
{
    mv_str = "";
    if (mv_piece(move) == W_KING && mv_castle(move)) {
        if (mv_to(move) == G1) {
            strcat(mv_str, "O-O");
        } else {
            strcat(mv_str, "O-O-O");
        }
        return;
    }

    if (mv_piece(move) == B_KING && mv_castle(move)) {
        if (mv_to(move) == G8) {
            strcat(mv_str, "O-O");
        } else {
            strcat(mv_str, "O-O-O");
        }
        return;
    }

    //algebraic notation special for pawns
    if (mv_piece(move) == W_PAWN || mv_piece(move) == B_PAWN) {
        if (mv_cap(move)) {
            strcat(mv_str, FILE_STR[mv_from(move)]);
        }
    } else {
        strcat(mv_str, PIECE_STR[mv_piece(move)]);
    }

    if (mv_cap(move)) {
        strcat(mv_str, "x");
    }

    strcat(mv_str, SQ_STR[mv_to(move)]);

    if (mv_prom(move)) {
        strcat(mv_str, "=");
        strcat(mv_str, PIECE_STR[mv_prom(move)]);
    }
}
