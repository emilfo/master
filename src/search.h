#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"

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
} S_SEARCH_SETTINGS;

void search_position(S_BOARD *b, S_SEARCH_SETTINGS *ss);

#endif /* SEARCH_H */
