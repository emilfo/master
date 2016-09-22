#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "globals.h"
#include "data.h"
#include "hash.h"
#include "search.h"
#include "threads.h"
#include "uci.h"
#include "perft.h"
#include "eval.h"
#include "io.h"
#include "bitops.h"

static void init_all(int thread_count) {
    init_data();
    init_hash();
    init_hashtable(0);
    init_board();
    init_threads(thread_count);
}

static void destroy_all() {
    destroy_hashtable();
    destroy_threads();
}

int main(int argc, char *argv[]) {
    int thread_count = 1;
    if (argc >= 2) {
        thread_count = atoi(argv[1]);
    }

    init_all(thread_count);

    if (argc == 3) {
        bench_file(argv[2]);
    } else {
        engine_shell();
    }

    destroy_all();

    printf("%s", QUOTE);

    return 0;
}
