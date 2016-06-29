#ifndef SEARCH_H
#define SEARCH_H

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

//int is_repetition(S_BOARD *b);

#endif /* SEARCH_H */
