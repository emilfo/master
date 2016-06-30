#include <stdio.h>
#include <inttypes.h>
#include "globals.h"
#include "movegen.h"
#include "io.h"
#include "perft.h"
#include "search.h"

#define BUG_FEN1 "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1"
#define BUG_FEN2 "n1Q5/P1Pk4/8/8/8/8/4Kppp/5N1N b  - 0 1"
#define BUG_FEN3 "nBn5/P1Pk4/8/8/8/8/4Kppp/5N1N b  - 0 1"
#define WAC1 "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - -"

int main() {
    printf("init data\n");

    init(&global_board);
    S_MOVELIST list[1];
    S_SEARCH_SETTINGS ss[1];


    parse_fen(&global_board, WAC1);
    
    char input[7];
    int move;
    while (true) {
        generate_all_moves(&global_board, list);
        
        printf("hash_key %"PRIx64"\n", global_board.hash_key);
        printf("action >");
        fgets(input, 7, stdin);

        if (input[0] == 'u') {
            unmake_move(&global_board);
        } else if (input[0] == 's') {
            ss->depth = 4;
            search_position(&global_board, ss);
        } else if (input[0] == 'f') {
            perft_from_file("perftsuite.epd", false);
        } else if (input[0] == 'q') {
            break;
        } else if (input[0] == 'p') {
            print_board(&global_board);
            print_movelist(list);
        } else {
            move = str_move(input, &global_board);
            if(move) {
                hash_put(&tp_table, global_board.hash_key, move, 0, 0);
                make_move(&global_board, move); 
            } else {
                printf("move not valid\n");
            }
        }
    }
    

    printf("\n\"You know somethin', Utivich? I think this just might be my mast"
            "erpiece.\"\n- Lt. Aldo Raine\n");

    return 0;
}
