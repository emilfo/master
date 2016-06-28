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

int main() {
    printf("init data\n");

    init();
   // print_board();
   // assert(debug_board());

    S_MOVELIST list[1];

   // generate_all_moves(board, &list[0]);

   // print_movelist(&list[0]);

   // make_move(list[0].moves[31].move, &board);

   // print_board();

   // unmake_move(&board);

   // print_board(); 

    //perft_from_file("perftsuite.epd", false);
    
    //perft_fen(BUG_FEN3, true, 1);
    
    char input[7];
    int move;
    while (true) {
        generate_all_moves(board, list);
        
        printf("hash_key %"PRIx64"\n", board.hash_key);
        printf("action >");
        fgets(input, 7, stdin);

        if (input[0] == 'u') {
            unmake_move(&board);
        } else if (input[0] == 't') {
            int moves[4];
            int i;
            hash_get_pv_line(&tp_table, &board, moves, 4);

            for(i=0; i < 4; i++) {
                printf("%s ", move_str(moves[i]));
            }
            printf("\n");
        } else if (input[0] == 'q') {
            break;
        } else if (input[0] == 'p') {
            print_board();
            print_movelist(list);
        } else {
            move = str_move(input, &board);
            if(move) {
                make_move(&board, move); 
                hash_put(&tp_table, board.hash_key, move, 0, 0);
            } else {
                printf("move not valid\n");
            }
        }

        if (is_repetition(&board)) {
            printf("IS REPETITION\n");
        }
    }
    

    printf("\n\"You know somethin', Utivich? I think this just might be my mast"
            "erpiece.\"\n- Lt. Aldo Raine\n");

    return 0;
}
