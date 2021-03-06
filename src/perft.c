#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include "globals.h"
#include "globals.h"
#include "board.h"
#include "perft.h"
#include "movegen.h"
#include "io.h"
#include "utils.h"
#include "eval.h"

static int perft(S_BOARD *b, int depth);

void perft_fen(char *FEN, int divide, int depth) {
    int node_cnt = 0;

    parse_fen(&g_board, FEN);

    printf("Perft from this position:\n");
    print_board(&g_board);

    if (divide) {
        node_cnt = perft_divide(&g_board, depth);
    } else {
        node_cnt = perft(&g_board, depth);
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


    long starttime = cur_time_millis();
    while (fgets(buf, 1024, fp) != NULL) {
        FILE *fp_result = fopen("LOG/perft-result.txt", "a");
        fprintf(fp_result, "%s", buf);

        parse_fen(&g_board, buf);

        printf("Perft from this position:\n");
        print_board(&g_board);

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
                    node_cnt = perft_divide(&g_board, depth);
                } else {
                    node_cnt = perft(&g_board, depth);
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
        long totaltime = cur_time_millis() - starttime;
        fprintf(fp_result, "\nTOTAL TIME USED: %ld\n\n", totaltime);
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


int perft_divide(S_BOARD *b, int depth) {
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

/**
 * Tests that the eval function works the same for both sides. evals a postion,
 * mirrors the board then eval the same position mirrored (same results is 
 * expected)
 * Doesn't really belong here, but figured it would clutter the eval.c file
 * and is basically same as perft_from_file
 */
void eval_from_file(const char *filename)
{
    FILE *fp = fopen(filename, "r");


    char buf[1024];
    int orig_eval = 0;
    int flip_eval = 0;

    long starttime = cur_time_millis();
    while (fgets(buf, 1024, fp) != NULL) {
        FILE *fp_result = fopen("LOG/eval-result.txt", "a");
        fprintf(fp_result, "%s", buf);

        parse_fen(&g_board, buf);

        printf("Eval test from this position:\n");

        print_board(&g_board);
        orig_eval = eval_posistion(&g_board);

        flip_board(&g_board);

        print_board(&g_board);
        flip_eval = eval_posistion(&g_board);

        if(orig_eval == flip_eval) {
            printf("orig_eval = flip_eval =%d - ok,\n", orig_eval);
            fprintf(fp_result, "orig_eval = flip_eval =%d - ok,\n", orig_eval);
        } else {
            printf("orig_eval = %d != flip_eval =%d\n", orig_eval, flip_eval);
            fprintf(fp_result, "orig_eval = %d != flip_eval =%d\n", orig_eval, flip_eval);
            fclose(fp_result);
            break;
        }

        assert(orig_eval == flip_eval);

        long totaltime = cur_time_millis() - starttime;
        fprintf(fp_result, "\nTOTAL TIME USED: %ld\n\n", totaltime);
        fclose(fp_result);
    }
}

void rating_from_file(const char *filename)
{
    FILE *fp = fopen(filename, "r");


    char buf[1024];
    char *ptr;
    char move[6];
    int i;

    while (fgets(buf, 1024, fp) != NULL) {
        wait_search_complete_barrier();
        threads_currently_searching = false;

        parse_fen(&g_board, buf);

        ptr = strstr(buf, "bm");
        ptr += 3;
        i = 0;
        do {
            if (*ptr != ';')
                move[i++] = *ptr;
        } while (*ptr++);
        move[i] = '\0';

        printf("RATING Testing this position:\n");

        print_board(&g_board);
        printf("\nMove to find: %s\n", move);
        g_search_info.depth = MAX_PLY;
        g_search_info.depth_set = false;
        g_search_info.time_set = true;
        g_search_info.stop = false;
        g_search_info.starttime = cur_time_millis();
        g_search_info.stoptime = g_search_info.starttime + 900000;

        //printf("time:%d start:%d stop:%d depth:%d timeset:%d\n", time, ss->starttime, ss->stoptime, ss->depth, ss->time_set);

        start_threads();
    }

    fclose(fp);
}

void bench_file(const char *filename)
{
    bench_file_depth(filename, 6);
}

void bench_file_depth(const char *filename, int depth)
{
    FILE *fp = fopen(filename, "r");

    char buf[1024];

    unsigned long cumulative_time = 0;
    long cumulative_nodes = 0L;
    unsigned long start_time;
    int position = 1;

    stop_threads();
    while (fgets(buf, 1024, fp) != NULL) {

        parse_fen(&g_board, buf);
        g_search_info.depth = depth;
        g_search_info.time_set = false;
        g_search_info.starttime = cur_time_millis();
        g_search_info.stoptime = 0;
        g_search_info.stop = false;

        //printf("time:%d start:%d stop:%d depth:%d timeset:%d\n", time, ss->starttime, ss->stoptime, ss->depth, ss->time_set);

	printf("position %d\n", position++);
        start_time = cur_time_millis();
        start_threads();

        wait_search_complete_barrier();
        threads_currently_searching = false;
        cumulative_time += cur_time_millis() - start_time;
        cumulative_nodes += count_all_nodes();
        clear_hashtable();

    }
    fclose(fp);

    printf("|         bench done         \n");
    printf("|----------------------------\n");
    printf("|time  : %ld\n", cumulative_time);
    printf("|nodes : %ld\n", cumulative_nodes);

}
