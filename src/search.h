#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"
#include "movegen.h"

typedef struct {
    int quit;
    int stop;

    int depth;
    int depth_set;
    
    int starttime;
    int stoptime;
    int time_set;
    int infinite;

    long nodes;

    float fail_high;
    float first_fail_high;
} S_SEARCH_SETTINGS;

void search_position(S_BOARD *b, S_SEARCH_SETTINGS *ss);
void set_best_move_next(int start_index, S_MOVELIST *l);

#endif /* SEARCH_H */
