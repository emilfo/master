#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "globals.h"
#include "board.h"
#include "hashtable.h"
#include "io.h"

u64 tp_size = ((u64)0x1000000 * (u64)50); //16MB size TP_TABLE (TODO: not static)


static void clear_hashtable(S_HASHTABLE *tp) 
{
    int i;
    fail_checksum = 0;

    for(i = 0; i < tp->size; i++) {
        tp->entries[i].hash_key = 0LL;
        tp->entries[i].move = 0;
        tp->entries[i].eval = 0;
        tp->entries[i].flag_and_age = 0;
        tp->entries[i].checksum = 0;
    }
}

void init_hashtable(S_HASHTABLE *tp, u64 size)
{
    tp->size = size / sizeof(S_HASHENTRY);
    tp->cut = 0;

    destroy_hashtable(tp);

    tp->entries = (S_HASHENTRY *) malloc(tp->size * sizeof(S_HASHENTRY));

    clear_hashtable(tp);

    printf("init hashtable with %d entries\n", tp->size);
}

void destroy_hashtable(S_HASHTABLE *tp)
{
    if (tp->entries != NULL) {
        printf("fail_checksums:%d",fail_checksum);
        printf("hash cuts:%d",tp->cut);
        free(tp->entries);
    }
}

int probe_hash(const S_HASHTABLE *tp, u64 key, S_HASHENTRY *entry, int *score, int alpha, int beta, int depth)
{
    if (hash_get(tp, key, entry)) {
        if (entry->depth >= depth) {
            if (entry->flag_and_age & EXCA_FLAG) {
                *score = entry->eval;
                return true;
            }
            if ((entry->flag_and_age & ALPH_FLAG) && (entry->eval <= alpha)) {
                *score = alpha;
                return true;
            }
            if ((entry->flag_and_age) & BETA_FLAG && (entry->eval >= beta)) {
                *score = beta;
                return true;
            }
        }
    }
    return false;
}

int hash_get(const S_HASHTABLE *tp, u64 key, S_HASHENTRY *entry)
{
    int i = key % tp->size;

    if (tp->entries[i].hash_key == key) {
        *entry = tp->entries[i];

        //Only return if checksum is OK
        int checksum = entry->hash_key ^ entry->move ^ entry->eval ^ entry->depth ^ entry->flag_and_age;
        if(entry->checksum == checksum) {
            return 1;
        }
        fail_checksum++;
    }

    entry = NULL;
    return 0;
}

void hash_put(S_HASHTABLE *tp, u64 key, uint32_t move, int16_t eval, uint8_t depth, int16_t age, int16_t flag)
{
    int i = key % tp->size;

    uint16_t flag_and_age = (age & AGE_MASK) | (flag & FLAG_MASK);
    if(flag & BETA_FLAG) {
        tp->entries[i].beta= true;
        tp->entries[i].alpha= false;
        tp->entries[i].exca = false;
    }

    //resetting eval to mate-score (ignoring moves to mate)
    if (eval > ISMATE) {
        eval = MATE;
    } else if (eval < -ISMATE) {
        eval = -MATE;
    }

    int32_t checksum = key ^ move ^ eval ^ depth ^ flag_and_age;

    tp->entries[i].hash_key = key;
    tp->entries[i].move = move;
    tp->entries[i].eval = eval;
    tp->entries[i].depth = depth;
    tp->entries[i].flag_and_age = flag_and_age;
    tp->entries[i].checksum = checksum;
}

int hash_get_pv_line(const S_HASHTABLE *tp, S_BOARD *b, int *moves, int depth)
{
    int i = 0;
    int j = 0;

    S_HASHENTRY entry;
    hash_get(tp, b->hash_key, &entry);

    while(i < depth && hash_get(tp, b->hash_key, &entry)) {
        if(make_move_if_exist(b, entry.move)) {
            moves[i++] = entry.move;
        } else {
            break;
        }
    }

    while(j++ < i) {
        unmake_move(b);
    }

    return i;
}
