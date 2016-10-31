#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "defs.h"
#include "uci.h"
#include "utils.h"
#include "io.h"
#include "search.h"
#include "threads.h"
#include "globals.h"

#define MAX_INPUT_SIZE 4096
#define NOT_SET 0

static void uci_identify() 
{
    printf("id name %s\n", NAME);
    printf("id author %s\n", AUTHOR);

    //UCI-options
    printf("\n");
    printf("option name Hash type spin default %d min %d max %d\n",
            HASH_DEF, HASH_MIN, HASH_MAX);
    printf("option name Threads type spin default %d min %d max %d\n",
            THREADS_DEF, THREADS_MIN, THREADS_MAX);

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
    u64 movetime = NOT_SET;
    u64 time = NOT_SET;
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
    ss->stop = false;
    ss->quit = false;

    printf("time:%"PRIu64" start:%"PRIu64" stop:%"PRIu64" depth:%d timeset:%d\n", time, ss->starttime, ss->stoptime, ss->depth, ss->time_set);
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
                stop_threads();
                clear_hashtable();
                hard_reset_board(&g_board);
            } else if(strncmp(input, "setoption", 9) == 0) {
                stop_threads();
                //TODO
            } else if(strncmp(input, "position", 8) == 0) {
                stop_threads();
                parse_position(input, &g_board);
            }else if(strncmp(input, "isready", 7) == 0) {
                printf("readyok\n");
            } else if(strncmp(input, "stop", 4) == 0) {
                stop_threads();
            } else if(strncmp(input, "quit", 4) == 0) {
                break;
            } else if(strncmp(input, "uci", 3) == 0) {
                uci_identify();
            } else if(strncmp(input, "go", 2) == 0) {
                stop_threads();
                parse_go(input, &g_search_info, &g_board);
                start_threads();
            } else {
                printf("error parsing input: %s\n", input);
            }
        }
    }
}

