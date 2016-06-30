#include <stdio.h>
#include "search.h"
#include "board.h"
#include "globals.h"
#include "eval.h"
#include "io.h"
#include "utils.h"

static int is_repetition(S_BOARD *b) 
{
    int i;

    for (i = 0; i < b->ply; i++) {
        if (b->hash_key == b->prev[i].hash_key) {
            return true;
        }
    }

    return false;
}

static void check_search_stop ()
{
}

static void prepare_search(S_BOARD *b, S_SEARCH_SETTINGS *ss)
{
    int i, j;

    ss->starttime = cur_time_millis();
    ss->stop = 0;
    ss->nodes = 0;

    for (i = 0; i < 13; i++) {
        for (j = 0; j < 64; j++) {
            b->search_history[i][j] = 0;
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < MAX_PLY; j++) {
            b->search_killers[i][j] = 0;
        }
    }

    b->search_ply = 0;
    
}

static int alpha_beta(S_BOARD *b, S_SEARCH_SETTINGS *ss, int alpha, int beta, int depth) //, int window) //TODO: window?
{
    return 0;
}

void search_position(S_BOARD *b, S_SEARCH_SETTINGS *ss)
{
    int best_moves[MAX_PLY];
    int best_score = -INFINITE;
    int cur_depth = 0;
    int pv_moves = 0;
    int i;

    prepare_search(b, ss);

    for (cur_depth = 1; cur_depth <= ss->depth; cur_depth++) {
        best_score = alpha_beta(b, ss, -INFINITE, INFINITE, cur_depth);
        pv_moves = hash_get_pv_line(&tp_table, b, best_moves, cur_depth);
        printf("depth:%d, move:%s, score:%d, nodes:%ld\n", cur_depth, move_str(best_moves[0]), best_score, ss->nodes);

        printf("pv: ");
        for(i=0; i < 4; i++) {
            printf("%s ", move_str(best_moves[i]));
        }
    }
}
