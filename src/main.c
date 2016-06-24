#include <stdio.h>
#include "globals.h"
#include "movegen.h"
//#include "io.h"
#include "perft.h"

#define BUG_FEN "nBn5/P1Pk4/8/8/8/8/4Kppp/5N1N b  - 0 1"
int main() {
    printf("init data\n");

    init();
   // print_board();
   // assert(debug_board());

   // S_MOVELIST list[1];

   // generate_all_moves(board, &list[0]);

   // print_movelist(&list[0]);

   // make_move(list[0].moves[31].move, &board);

   // print_board();

   // unmake_move(&board);

   // print_board(); 

    //perft_from_file("perftsuite.epd", true);
    
    perft_fen(BUG_FEN, true);
    

    printf("\n\"You know somethin', Utivich? I think this just might be my mast"
            "erpiece.\"\n- Lt. Aldo Raine\n");

    return 0;
}
