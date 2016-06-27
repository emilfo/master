#include "globals.h"

int is_repetition(S_BOARD *b) 
{
    int i;

    for (i = 0; i < b->ply; i++) {
        if (b->hash_key == b->prev[i].hash_key) {
            return true;
        }
    }

    return false;
}

