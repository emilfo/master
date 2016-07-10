#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"
#include "movegen.h"

typedef struct {
    int quit;
    int stop;
    //int depth_done;

    int depth;
    int depth_set;
    int cur_depth;

    int starttime;
    int stoptime;
    int time_set;
    int infinite;

    volatile long nodes;

    float fail_high;
    float first_fail_high;
} S_SEARCH_SETTINGS;

void search_position(S_BOARD *b, S_SEARCH_SETTINGS *ss);
void set_best_move_next(int start_index, S_MOVELIST *l);
void make_move_and_search(S_BOARD *b, S_SEARCH_SETTINGS *ss, S_MOVELIST *l, int move_index, int alpha, int beta, int depth);
void start_new_search(S_BOARD *b, S_SEARCH_SETTINGS *ss);

#endif /* SEARCH_H */
