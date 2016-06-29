#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "movegen.h"
#include "io.h"

char *sq_str(const int sq) {
    static char str[3];

    sprintf(str, "%c%c", ('a'+files[sq]-1), ('1'+ranks[sq]-1));
    return str;
}

char *move_str(const uint32_t move) {
    static char str[7];

    //sprintf(str, "%c", PIECE_PRINT[mv_piece(move)]); //Piece (if not pawn)
    sprintf(str, "%s", sq_str(mv_from(move))); //fromsq
    //sprintf(&str[strlen(str)], "%c", CAP_PRINT[mv_cap(move)]); //x if a capture
    sprintf(&str[strlen(str)], "%s", sq_str(mv_to(move))); //tosq
    sprintf(&str[strlen(str)], "%c", PIECE_PRINT[mv_prom(move)]); //promoted (if any)

    return str;
}

int str_sq(const char *sq) {
    assert(sq[0] >= 'a' && sq[0] <= 'h');
    assert(sq[1] >= '1' && sq[1] <= '8');

    return FRtoSQ(sq[0]-'a'+1, sq[1]-'1'+1);
}

int str_move(const char *sq, S_BOARD *b) {
    int from = str_sq(sq);
    int to = str_sq(sq+2);
    int prom;
    int i;
    int move;

    switch (sq[4]) {
        case 'N':
            prom = (b->side)? B_KNIGHT : W_KNIGHT;
            break;
        case 'B':
            prom = (b->side)? B_BISHOP : W_BISHOP;
            break;
        case 'R':
            prom = (b->side)? B_ROOK : W_ROOK;
            break;
        case 'Q':
            prom = (b->side)? B_QUEEN : W_QUEEN;
            break;
        default:
            prom = EMPTY;
            break;
    }

    assert(valid_sq(from));
    assert(valid_sq(to));
    assert(valid_piece_or_empty(prom));

    S_MOVELIST list[1];
    generate_all_moves(b, list);

    for(i = 0; i < list->index; i++) {
        move = list->moves[i].move;
        if (mv_to(move) == to && mv_from(move) == from && mv_prom(move) == prom) {
            return move;
        }
    }

    return EMPTY;
}

void print_movelist(S_MOVELIST *list)
{
    int i;
    S_MOVE m;

    printf("\nmovelist (%d moves):\n", list->index);
    for (i = 0; i < list->index; i++) {
        m = list->moves[i];
        printf("Move no. %d: %s, score:%d\n", i, move_str(m.move), m.score);
    }
}
