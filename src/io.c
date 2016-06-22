#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "movegen.h"

const static char PIECE_PRINT[13] = { '\0',
    '\0', 'N', 'B', 'R', 'Q', 'K',
    '\0', 'N', 'B', 'R', 'Q', 'K'
};

const static char CAP_PRINT[13] = { '\0',
    'x', 'x', 'x', 'x', 'x', 'x',
    'x', 'x', 'x', 'x', 'x', 'x'
};

char *sq_str(const int sq) {
    static char str[3];

    sprintf(str, "%c%c", ('a'+files[sq]-1), ('1'+ranks[sq]-1));
    return str;
}

char *move_str(const uint32_t move) {
    static char str[7];

    sprintf(str, "%c", PIECE_PRINT[mv_piece(move)]); //Piece (if not pawn)
    sprintf(&str[strlen(str)], "%s", sq_str(mv_from(move))); //fromsq
    sprintf(&str[strlen(str)], "%c", CAP_PRINT[mv_cap(move)]); //x if a capture
    sprintf(&str[strlen(str)], "%s", sq_str(mv_to(move))); //tosq
    sprintf(&str[strlen(str)], "%c", PIECE_PRINT[mv_prom(move)]); //promoted (if any)

    return str;
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
