#ifndef IO_H
#define IO_H

#include "movegen.h"

const static char PIECE_PRINT[13] = { '\0',
    '\0', 'n', 'b', 'r', 'q', 'k',
    '\0', 'n', 'b', 'r', 'q', 'k'
};

const static char CAP_PRINT[13] = { '\0',
    'x', 'x', 'x', 'x', 'x', 'x',
    'x', 'x', 'x', 'x', 'x', 'x'
};

void print_movelist(S_MOVELIST *list);
char *move_str(const uint32_t move);
int str_move(const char *sq, S_BOARD *b);
void engine_shell(); 

#endif /* IO_H */
