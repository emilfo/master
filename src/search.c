#include "search.h"
#include "globals.h"
#include "eval.h"

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
}

static int alpha_beta(S_BOARD *b, S_SEARCH_SETTINGS *ss, int alpha, int beta, int depth, int window) //TODO: window?
{
    return 0;
}

