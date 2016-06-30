#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "hashtable.h"
#include "io.h"

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

S_HASHENTRY *hash_get(const S_HASHTABLE *tp, u64 key)
{
    int i = key % tp->size;

    //printf("GET entry[%d]\n", i);
    //TODO: lock here?
    if (tp->entries[i].hash_key == key) {
        return &tp->entries[i];
    }

    return NULL;
}

void hash_put(S_HASHTABLE *tp, u64 key, uint32_t move, int16_t eval, int16_t age)
{
    int i = key % tp->size;

    //printf("PUT entry[%d]: %s\n", i, move_str(move));

    //TODO: lock here?
    tp->entries[i].hash_key = key;
    tp->entries[i].move = move;
    tp->entries[i].eval = eval;
    tp->entries[i].age = age;
}

int hash_get_pv_line(const S_HASHTABLE *tp, S_BOARD *b, int *moves, int depth)
{
    int i = 0;
    int j = 0;

    S_HASHENTRY *move = hash_get(tp, b->hash_key);

    while(i < depth && move != NULL) {
        if(make_move_if_exist(b, move->move)) {
            moves[i++] = move->move;
        } else {
            break;
        }
        move = hash_get(tp, b->hash_key);
    }

    while(j++ < i) {
        unmake_move(b);
    }

    return i;
}
