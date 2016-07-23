#ifndef IO_H
#define IO_H

#include "defs.h"
#include "movegen.h"

static const char PIECE_PRINT[13] = { '\0',
    '\0', 'n', 'b', 'r', 'q', 'k',
    '\0', 'n', 'b', 'r', 'q', 'k'
};

static const char CAP_PRINT[13] = { '\0',
    'x', 'x', 'x', 'x', 'x', 'x',
    'x', 'x', 'x', 'x', 'x', 'x'
};

void print_movelist(S_MOVELIST *list);
char *move_str(const u32 move);
int str_move(const char *sq, S_BOARD *b);
void engine_shell(); 

#endif /* IO_H */
