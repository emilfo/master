#include <stdio.h>
#include "search.h"
#include "board.h"
#include "globals.h"
#include "eval.h"
#include "io.h"
#include "utils.h"

static int is_repetition(S_BOARD *b) 
{
    int i;

    for (i = 0; i < b->ply; i++) {
        if (b->hash_key == b->prev[i].hash_key) {
            return true;
        }
    }

    return false;
}

//static void check_search_stop ()
//{
//}

static void prepare_search(S_BOARD *b, S_SEARCH_SETTINGS *ss)
{
    int i, j;

    ss->starttime = cur_time_millis();
    ss->stop = 0;
    ss->nodes = 0;
    ss->fail_high = 0;
    ss->first_fail_high = 0;

    for (i = 0; i < 13; i++) {
        for (j = 0; j < 64; j++) {
            b->search_history[i][j] = 0;
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < MAX_PLY; j++) {
            b->search_killers[i][j] = 0;
        }
    }

    b->search_ply = 0;
    
}

static void set_best_move_next(int start_index, S_MOVELIST *l)
{
    int i;
    int best_index = start_index;
    int best_score = 0;

    for (i = start_index; i < l->index; i++) {
        if (l->moves[i].score > best_score) {
            best_score = l->moves[i].score;
            best_index = i;
        }
    }
    S_MOVE swp = l->moves[start_index];
    l->moves[start_index] = l->moves[best_index];
    l->moves[best_index] = swp;
}

static int alpha_beta(S_BOARD *b, S_SEARCH_SETTINGS *ss, int alpha, int beta, int depth) //, int window) //TODO: window?
{
    int i;
    int legal = 0;
    int old_alpha = alpha;
    int best_move = EMPTY;
    int score = -INFINITE;

    assert(debug_board(b));

    ss->nodes++;

    if (depth == 0) {
        return eval_posistion(b);
    }

    if (b->search_ply && (is_repetition(b) || b->fifty_move_count >= 100)) {
        return 0;
    }

    if (b->search_ply >= MAX_PLY) {
        return eval_posistion(b);
    }

    S_MOVELIST l[1];
    generate_all_moves(b, l);

    for (i = 0; i < l->index; i++) {

        set_best_move_next(i, l);
        //if(depth == 5) {
        //    printf("testing move %s\n", move_str(l->moves[i].move));
        //}
        if (make_move(b, l->moves[i].move)) {
            b->search_ply++;

            score = -alpha_beta(b, ss, -beta, -alpha, depth-1);
            unmake_move(b);
            b->search_ply--;

            if (score > alpha) {
                if (score >= beta) {
                    ss->fail_high++;
                    if (legal == 0) {
                        ss->first_fail_high++;
                    }
                    return beta;
                }

                alpha = score;
                best_move = l->moves[i].move;
            }

            legal++;
        }
    }

    if (legal == 0) {
        //printf("legal=0, k-index:%d\n", KING_INDEX[b->side]);
        if (sq_attacked(b, lsb1_index(b->piece_bb[KING_INDEX[b->side]]), 1-b->side)) {
            return -MATE + b->search_ply;
        } else {
          return 0;
        }
    }

    if (alpha != old_alpha) {
        hash_put(&tp_table, b->hash_key, best_move, score, b->ply);
    }


    return score;
}

void search_position(S_BOARD *b, S_SEARCH_SETTINGS *ss)
{
    int best_moves[MAX_PLY];
    int best_score = -INFINITE;
    int cur_depth = 0;
    int pv_moves = 0;
    int i;

    prepare_search(b, ss);

    for (cur_depth = 1; cur_depth <= ss->depth; cur_depth++) {
        best_score = alpha_beta(b, ss, -INFINITE, INFINITE, cur_depth);

        pv_moves = hash_get_pv_line(&tp_table, b, best_moves, cur_depth);
        printf("depth:%d, move:%s, score:%d, nodes:%ld\n", cur_depth, move_str(best_moves[0]), best_score, ss->nodes);

        printf("pv: ");
        for(i=0; i < pv_moves; i++) {
            printf("%s ", move_str(best_moves[i]));
        }
        printf("\n");
        printf("Ordering: %.1f/%.1f = %.3f\n",ss->first_fail_high,ss->fail_high, ss->first_fail_high/ss->fail_high);
    }
}
