#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "globals.h"
#include "data.h"
#include "hash.h"
#include "search.h"
#include "threads.h"
#include "uci.h"
#include "eval.h"
#include "io.h"
#include "bitops.h"

static void init_all() {
    init_data();
    init_hash();
    init_hashtable(&global_tp_table, tp_size);
    init_board(&global_board);
    init_threads(16);
}

static void destroy_all() {
    destroy_hashtable(&global_tp_table);
    destroy_threads();
}

int main() {
    init_all();

    engine_shell();

    destroy_all();

    printf("%s", QUOTE);
    return 0;
}
