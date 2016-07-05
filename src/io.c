#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "globals.h"
#include "movegen.h"
#include "io.h"
#include "uci.h"
#include "perft.h"

char *sq_str(const int sq) {
    static char str[3];

    sprintf(str, "%c%c", ('a'+files[sq]-1), ('1'+ranks[sq]-1));
    return str;
}

char *move_str(const uint32_t move) {
    static char str[7];
    int prom = mv_prom(move);

    sprintf(str, "%s", sq_str(mv_from(move))); //fromsq
    sprintf(&str[strlen(str)], "%s", sq_str(mv_to(move))); //tosq
    if (prom) {
        sprintf(&str[strlen(str)], "%c", PIECE_PRINT[mv_prom(move)]); //promoted (if any)
    }

    return str;
}

int str_sq(const char *sq) {
    assert(sq[0] >= 'a' && sq[0] <= 'h');
    assert(sq[1] >= '1' && sq[1] <= '8');

    return FRtoSQ(sq[0]-'a'+1, sq[1]-'1'+1);
}

int str_move(const char *sq, S_BOARD *b) {
    int from = str_sq(sq);
    int to = str_sq(sq+2);
    int prom = EMPTY;
    int i;
    int move;

    if (strlen(sq) > 4) {
        switch (sq[4]) {
            case 'n':
                prom = (b->side)? B_KNIGHT : W_KNIGHT;
                break;
            case 'b':
                prom = (b->side)? B_BISHOP : W_BISHOP;
                break;
            case 'r':
                prom = (b->side)? B_ROOK : W_ROOK;
                break;
            case 'q':
                prom = (b->side)? B_QUEEN : W_QUEEN;
                break;
            default:
                prom = EMPTY;
                break;
        }
    }

    assert(valid_sq(from));
    assert(valid_sq(to));
    assert(valid_piece_or_empty(prom));

    S_MOVELIST list[1];
    generate_all_moves(b, list);

    for(i = 0; i < list->index; i++) {
        move = list->moves[i].move;
        if (mv_to(move) == to && mv_from(move) == from && mv_prom(move) == prom) {
            return move;
        }
    }

    return EMPTY;
}

void print_movelist(S_MOVELIST *list)
{
    int i;
    S_MOVE m;

    printf("\nmovelist (%d moves):\n", list->index);
    for (i = 0; i < list->index; i++) {
        m = list->moves[i];
        printf("Move no. %d: %s, score:%d\n", i, move_str(m.move), m.score);
    }
}

void engine_shell() 
{
    char input[256];

    while (true) {
        printf(">");
        fflush(stdout);

        fgets(input, 256, stdin);

        if(strncmp(input, "file-perft", 10) == 0) {
            perft_from_file("EPD/perftsuite.epd", false);
        } else if(strncmp(input, "ratingtest", 10) == 0) {
            rating_from_file("EPD/BT2450.epd");
        } else if(strncmp(input, "file-eval", 9) == 0) {
            eval_from_file("EPD/ecm98.epd");
        } else if(strncmp(input, "divide", 6) == 0) {
            perft_divide(&global_board, atoi(input+7));
        } else if(strncmp(input, "mirror", 6) == 0) {
            flip_board(&global_board);
        } else if(strncmp(input, "print", 5) == 0) {
            print_board(&global_board);
        } else if(strncmp(input, "quit", 4) == 0) {
            global_search_settings.stop = true;
            global_search_settings.quit = true;
            thread_search_go();
            break;
        } else if(strncmp(input, "move", 4) == 0) {
            make_move_if_exist(&global_board, str_move(input+5, &global_board));
        } else if(strncmp(input, "fen", 3) == 0) {
            parse_fen(&global_board, input+4);
        } else if(strncmp(input, "uci", 3) == 0) {
            uci_loop();
            break;
        } else if(strncmp(input, "h", 1) == 0) {
            printf("\n\ncommands:\n");
            printf("file-perft\n");
            printf("fen\n");
            printf("mirror\n");
            printf("print\n");
            printf("quit\n");
            printf("move\n");
            printf("uci\n");
            printf("h\n");
        } else {
            printf("h for help\n");
        }
    }
}
