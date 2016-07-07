#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "uci.h"
#include "utils.h"
#include "io.h"
#include "search.h"
#include "globals.h"
#include "threads.h"

#define MAX_INPUT_SIZE 4096
#define NOT_SET -1

static void uci_identify() 
{
    printf("id name %s\n", NAME);
    printf("id auhor %s\n", AUTHOR);
    printf("uciok\n");
}

static void parse_position(char *input, S_BOARD *b)
{
    int move;
    input += 9; //skip "position "

    if (strncmp(input, "startpos", 8) == 0) {
        parse_fen(b, START_FEN);
    } else if (strncmp(input, "fen", 3) == 0) {
        input += 4; //skip "fen "
        parse_fen(b, input);
    } else {
        printf("position error\n");
        return;
    }

    input = strstr(input, "moves");
    if (input != NULL) {
        input += 6; //skip "moves "
    }

    while (input != NULL && strlen(input) >= 4) {

        move = str_move(input, b);
        if (move == EMPTY) {
            printf("COULDNT PARSE MOVE!!!\n");
            break;
        }

        make_move(b, move);

        input = strstr(input, " ");
        if (input != NULL) {
            input++; //skip " "
        }
    }

   // print_board(b);
}

static void parse_go(char *input, S_SEARCH_SETTINGS *ss, S_BOARD *b)
{
    char *buf_ptr = NULL;
    int depth = MAX_PLY;
    int movestogo = 30; //TODO? time management
    int movetime = NOT_SET;
    int time = NOT_SET;
    int inc = 0;
    ss->time_set = false;

    // PARSING INPUT
    if ((buf_ptr = strstr(input, "infinite"))) {
        ss->time_set = false;
    }
    if ((buf_ptr = (strstr(input, "binc"))) && b->side) {
        inc = atoi(buf_ptr + 5);
    }
    if ((buf_ptr = (strstr(input, "winc"))) && !b->side) {
        inc = atoi(buf_ptr + 5);
    }
    if ((buf_ptr = (strstr(input, "btime"))) && b->side) {
        time = atoi(buf_ptr + 6);
    }
    if ((buf_ptr = (strstr(input, "wtime"))) && !b->side) {
        time = atoi(buf_ptr + 6);
    }
    if ((buf_ptr = (strstr(input, "movestogo")))) {
        movestogo = atoi(buf_ptr + 10);
    }
    if ((buf_ptr = (strstr(input, "movetime")))) {
        movetime = atoi(buf_ptr + 9);
    }
    if ((buf_ptr = (strstr(input, "depth")))) {
        depth = atoi(buf_ptr + 6);
    }


    //Setting settings parsed
    ss->depth = depth;
    ss->cur_depth = 0;

    if (movetime != NOT_SET) {
        time = movetime;
        movestogo = 1;
    }

    ss->starttime = cur_time_millis();
    if(time != NOT_SET) {
        ss->time_set = true;
        time /= movestogo;

        time -= 50; //failsafe, don't want to run out
        ss->stoptime = ss->starttime + time + inc;
    }

    printf("time:%d start:%d stop:%d depth:%d timeset:%d\n", time, ss->starttime, ss->stoptime, ss->depth, ss->time_set);

    pthread_mutex_unlock(&report_move);
    signal_threads();
}

void uci_loop() 
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    char input[MAX_INPUT_SIZE];

    uci_identify();

    while (true) {
        memset(input, 0, sizeof(input));
        fflush(stdout);

        if(fgets(input, MAX_INPUT_SIZE, stdin) && !(input[0] == '\n')) {
            if(strncmp(input, "ucinewgame", 10) == 0) {
                hard_reset_board(&global_board);
                //parse_position("position startpos\n", &global_board);
            } else if(strncmp(input, "position", 8) == 0) {
                parse_position(input, &global_board);
            }else if(strncmp(input, "isready", 7) == 0) {
                printf("readyok\n");
            } else if(strncmp(input, "stop", 4) == 0) {
                global_search_settings.stop = true;
            } else if(strncmp(input, "quit", 4) == 0) {
                global_search_settings.stop = true;
                global_search_settings.quit = true;
            } else if(strncmp(input, "uci", 3) == 0) {
                uci_identify();
            } else if(strncmp(input, "go", 2) == 0) {
                parse_go(input, &global_search_settings, &global_board);
            } else {
                printf("error parsing input: %s\n", input);
            }
        }

        if(global_search_settings.quit) {
            break;
        }
    }
}

void uci_report_scores(S_BOARD *b, S_SEARCH_SETTINGS *ss, int score)
{
    int best_moves[MAX_PLY];
    int pv_moves = 0;
    int i;


    pv_moves = hash_get_pv_line(&global_tp_table, b, best_moves, ss->cur_depth);
    printf("info score cp %d depth %d nodes %ld time %d", score, ss->cur_depth, ss->nodes, cur_time_millis() - ss->starttime);

    global_bestmove = best_moves[0];

    printf(" pv");
    for(i=0; i < pv_moves; i++) {
        printf(" %s", move_str(best_moves[i]));
    }
    printf("\n");
    //printf("Ordering: %.1f/%.1f = %.3f\n",ss->first_fail_high,ss->fail_high, ss->first_fail_high/ss->fail_high);
}

void uci_print_bestmove()
{
    printf("bestmove %s\n", move_str(global_bestmove));
}
