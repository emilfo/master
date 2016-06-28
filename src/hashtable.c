#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "hashtable.h"

u64 tp_size = (0x1000000 * 2); //2MB size TP_TABLE (TODO: not static)


static void clear_hashtable(S_HASHTABLE *tp) 
{
    int i;

    for(i = 0; i < tp->size; i++) {
        tp->entries[i].hash_key = 0LL;
        tp->entries[i].move = 0;
        tp->entries[i].eval = 0;
        tp->entries[i].age = 0;
    }
}

void init_hashtable(S_HASHTABLE *tp, u64 size)
{
    tp->size = size / sizeof(S_HASHENTRY);

    if (tp->entries != NULL) {
        free(tp->entries);
    }
    tp->entries = (S_HASHENTRY *) malloc(tp->size * sizeof(S_HASHENTRY));

    clear_hashtable(tp);
    
    printf("init hashtable with %d entries\n", tp->size);
}

