#ifndef SEARCH_H
#define SEARCH_H

#include "defs.h"
#include "board.h"
#include "movegen.h"

typedef struct {
    int quit;
    int stop;

    int depth;
    int depth_set;

    u64 starttime;
    u64 stoptime;
    int time_set;
    int infinite;

    //long nodes;

    //float fail_high;
    //float first_fail_high;
} S_SEARCH_SETTINGS;

static const int aspiration_window[4]  = {10, 50, 250, INFINITE};

void search_position(S_BOARD *b, int thread_id);
void set_best_move_next(int start_index, S_MOVELIST *l);
long count_all_nodes();

#endif /* SEARCH_H */
