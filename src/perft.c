#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "board.h"
#include "perft.h"
#include "movegen.h"
#include "io.h"

static int perft(S_BOARD *b, int depth);
static int perft_divide(S_BOARD *b, int depth);

void test_from_file(const char *filename, int divide) {
    FILE *fp = fopen(filename, "r");

    char buf[1024];
    int i = 0;
    int depth = 0;
    int target_cnt = 0;
    int node_cnt = 0;
    //char *fen[100];
    //int node_count[6];

    
    printf("test\n");

    while (fgets(buf, 1024, fp) != NULL) {
        parse_fen(buf);

        printf("TESTING THIS BOARD:\n");
        print_board();

        i = 0;
        while(buf[i]) {
            if(buf[i] == 'D') {
                depth = atoi(&buf[i+1]);
                target_cnt = atoi(&buf[i+3]);

                printf("depth=%d, target node count=%d\n",depth, target_cnt); 
                if (divide) {
                    node_cnt = perft_divide(&board, depth);
                } else {
                    node_cnt = perft(&board, depth);
                }
                printf("actual node count=%d\n\n", node_cnt);

                assert(target_cnt == node_cnt);
            }
            i++;
        }
    }
}

static int perft(S_BOARD *b, int depth) {
    if (depth == 0) {
        return 1;
    }

    S_MOVELIST list[1];
    int count = 0;
    int i;

    generate_all_moves(*b, list);

    for (i = 0; i < list->index; i++) {
        if(make_move(b, list->moves[i].move)) {
            count += perft(b, depth - 1);
            unmake_move(b);
        }
    }

    return count;
}


static int perft_divide(S_BOARD *b, int depth) {
    S_MOVELIST list[1];
    int count = 0;
    int div_count = 0;
    int i;

    generate_all_moves(*b, list);

    for (i = 0; i < list->index; i++) {
        if(make_move(b, list->moves[i].move)) {
            div_count = perft(b, depth - 1);
            count += div_count;

            printf("Move: %s, count: %d\n", move_str(list->moves[i].move), div_count);
            unmake_move(b);
        }
    }

    return count;
}
