#ifndef IO_H
#define IO_H

#include "movegen.h"

const static char PIECE_PRINT[13] = { '\0',
    '\0', 'N', 'B', 'R', 'Q', 'K',
    '\0', 'N', 'B', 'R', 'Q', 'K'
};

const static char CAP_PRINT[13] = { '\0',
    'x', 'x', 'x', 'x', 'x', 'x',
    'x', 'x', 'x', 'x', 'x', 'x'
};

void print_movelist(S_MOVELIST *list);
char *move_str(const uint32_t move);
int str_move(const char *sq, S_BOARD *b);

#endif /* IO_H */
