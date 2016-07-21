#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"
#include "movegen.h"

typedef struct {
    int quit;
    int stop;

    int depth;
    int depth_set;

    long starttime;
    long stoptime;
    int time_set;
    int infinite;

} S_SEARCH_SETTINGS;

const static int aspiration_window[4]  = {10, 50, 250, 32767};

void make_move_and_search(S_BOARD *b, S_SEARCH_SETTINGS *ss, S_MOVELIST *l, int move_index, int alpha, int beta, int depth, S_BOARD *my_b);
void search_position(S_BOARD *b, S_SEARCH_SETTINGS *ss, S_BOARD *my_b);
void set_best_move_next(int start_index, S_MOVELIST *l);

#endif /* SEARCH_H */
