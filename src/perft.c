#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "board.h"
#include "perft.h"
#include "movegen.h"
#include "io.h"

static int perft(S_BOARD *b, int depth);
static int perft_divide(S_BOARD *b, int depth);

void perft_fen(char *FEN, int divide, int depth) {
    int node_cnt = 0;

    parse_fen(&board, FEN);

    printf("Perft from this position:\n");
    print_board();

    if (divide) {
        node_cnt = perft_divide(&board, depth);
    } else {
        node_cnt = perft(&board, depth);
    }
    printf("total node count=%d\n\n", node_cnt);
}

void perft_from_file(const char *filename, int divide) {
    FILE *fp = fopen(filename, "r");


    char buf[1024];
    int i = 0;
    int depth = 0;
    int target_cnt = 0;
    int node_cnt = 0;
    //char *fen[100];
    //int node_count[6];

    
    while (fgets(buf, 1024, fp) != NULL) {
        FILE *fp_result = fopen("perft-result.txt", "a");
        fprintf(fp_result, "%s", buf);

        parse_fen(&board, buf);

        printf("Perft from this position:\n");
        print_board();

        printf("\n\n%5s || %9s | %9s |\n", "depth", "target", "count"); 
        printf("-------------------------------------\n"); 
        i = 0;
        while(buf[i]) {
            if(buf[i] == 'D') {
                depth = atoi(&buf[i+1]);
                target_cnt = atoi(&buf[i+3]);

                printf("%5d || %9d | ",depth, target_cnt); 
                fflush(stdout);
                if (divide) {
                    node_cnt = perft_divide(&board, depth);
                } else {
                    node_cnt = perft(&board, depth);
                }
                printf("%9d |\n", node_cnt);


                if(target_cnt == node_cnt) {
                    fprintf(fp_result, "D%d - OK, ", depth);
                } else {
                    fprintf(fp_result, "D%d - ERROR!\n\n", depth);
                    fclose(fp_result);
                    break;
                }


                assert(target_cnt == node_cnt);
            }
            i++;
        }
        fprintf(fp_result, "\n\n");
        fclose(fp_result);
    }
}

static int perft(S_BOARD *b, int depth) {
    if (depth == 0) {
        return 1;
    }

    S_MOVELIST list[1];
    int count = 0;
    int i;

    generate_all_moves(b, list);

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

    generate_all_moves(b, list);

    for (i = 0; i < list->index; i++) {
        //if(i==19) {
        if(make_move(b, list->moves[i].move)) {
            div_count = perft(b, depth - 1);
            count += div_count;

            printf("Move: %s, count: %d\n", move_str(list->moves[i].move), div_count);
            unmake_move(b);
        }
        //else {
        //    printf("MOVE IN CHECK: %s, %d\n", move_str(list->moves[i].move), list->moves[i].move);
        //}
    }
    //}
    printf("move count: %d\n", count);

    return count;
}
