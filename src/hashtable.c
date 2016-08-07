#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "board.h"
#include "hashtable.h"
#include "io.h"

S_HASHTABLE g_hash_table;

void clear_hashtable()
{
    g_hash_table.fail_checksum = 0;
    g_hash_table.cut = 0;

    memset(g_hash_table.entries, 0, g_hash_table.size);
}

void init_hashtable(u64 size)
{
    if (size == 0) {
        size = HASH_DEF * 1000000;
    }

    g_hash_table.size = size / sizeof(S_HASHENTRY);
    g_hash_table.cut = 0;

    destroy_hashtable();

    g_hash_table.entries = (S_HASHENTRY *) calloc(g_hash_table.size, sizeof(S_HASHENTRY));

    printf("init hashtable with %d entries\n", g_hash_table.size);
}

void destroy_hashtable()
{
    if (g_hash_table.entries != NULL) {
        printf("fail_checksums:%d",g_hash_table.fail_checksum);
        printf("hash cuts:%d",g_hash_table.cut);
        free(g_hash_table.entries);
    }
}

int probe_hash(u64 key, S_HASHENTRY *entry, i16 *score, i16 alpha, i16 beta, int depth)
{
    if (hash_get(key, entry)) {
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

int hash_get(u64 key, S_HASHENTRY *entry)
{
    int i = key % g_hash_table.size;

    if (g_hash_table.entries[i].hash_key == key) {
        *entry = g_hash_table.entries[i];

        //Only return if checksum is OK
        uint32_t local_checksum = entry->hash_key ^ entry->move ^ entry->eval ^ entry->depth ^ entry->flag_and_age;
        if(entry->checksum == local_checksum) {
            return 1;
        }
        fail_checksum++;
    }

    entry = NULL;
    return 0;
}

void hash_put(u64 key, u32 move, i16 eval, u8 depth, i16 age, i16 flag)
{
    int i = key % g_hash_table.size;

    u16 flag_and_age = (age & AGE_MASK) | (flag & FLAG_MASK);
    //if(flag & BETA_FLAG) {
    //    g_hash_table.entries[i].beta= true;
    //    g_hash_table.entries[i].alpha= false;
    //    g_hash_table.entries[i].exca = false;
    //}

    //resetting eval to mate-score (ignoring moves to mate)
    if (eval >  ISMATE) {
        eval = MATE;
    } else if (eval < -ISMATE) {
        eval = -MATE;
    }

    int32_t checksum = key ^ move ^ eval ^ depth ^ flag_and_age;

    g_hash_table.entries[i].hash_key = key;
    g_hash_table.entries[i].move = move;
    g_hash_table.entries[i].eval = eval;
    g_hash_table.entries[i].depth = depth;
    g_hash_table.entries[i].flag_and_age = flag_and_age;
    g_hash_table.entries[i].checksum = checksum;
}

int hash_get_pv_line(S_BOARD *b, u32 *moves, int depth)
{
    int i = 0;
    int j = 0;

    S_HASHENTRY entry;

    while(i < depth && hash_get(b->hash_key, &entry)) {
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
