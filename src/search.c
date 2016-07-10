#include <stdio.h>

#include "search.h"
#include "globals.h"
#include "eval.h"
#include "io.h"
#include "utils.h"
#include "movegen.h"
#include "board.h"

#define BLACK_MAJ(b) (b->piece_bb[B_BISHOP] || b->piece_bb[B_ROOK] || b->piece_bb[B_QUEEN])
#define WHITE_MAJ(b) (b->piece_bb[W_BISHOP] || b->piece_bb[W_ROOK] || b->piece_bb[W_QUEEN])

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

static void check_search_stop(S_SEARCH_SETTINGS *ss)
{
    if (ss->time_set && cur_time_millis() >= ss->stoptime) {
        ss->stop = true;
    }
}

static void prepare_search(S_BOARD *b, S_SEARCH_SETTINGS *ss)
{
    int i, j;

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

static void store_killer(S_BOARD *b, int move) 
{
    b->search_killers[1][b->search_ply] = b->search_killers[0][b->search_ply];
    b->search_killers[0][b->search_ply] = move;
}

static void update_history(S_BOARD *b, int piece, int to_sq, int depth)
{
    b->search_history[piece][to_sq] += depth;
}

void set_best_move_next(int start_index, S_MOVELIST *l)
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

static int quiescence(S_BOARD *b, S_SEARCH_SETTINGS *ss, int alpha, int beta)
{
    int i;
    int legal = 0;
    //int old_alpha = alpha;
    //int best_move = EMPTY;
    int score;
    int move;

    assert(debug_board(b));

    //printf("WORKER THREAD quiescence\n");

    if (ss->nodes & 4095) {
        check_search_stop(ss);
    }

    ss->nodes++;

    if (is_repetition(b) || b->fifty_move_count >= 100) {
        return 0;
    }

    if (b->search_ply >= MAX_PLY) {
        score = eval_posistion(b);
        return score;
    }

    score = eval_posistion(b);

    if (score >= beta) {
        return beta;
    }

    if (score > alpha) {
        alpha = score;
    }

    S_MOVELIST l[1];
    generate_all_captures(b, l);

    score = -INFINITE;

    for (i = 0; i < l->index; i++) {

        set_best_move_next(i, l);
        move = l->moves[i].move;
        if (make_move(b, move)) {
            b->search_ply++;

            score = -quiescence(b, ss, -beta, -alpha);

            unmake_move(b);
            b->search_ply--;

            if (ss->stop) {
                return 0;
            }

            if(ss->stop) {
                return 0;
            }

            if (score > alpha) {
                if (score >= beta) {
                    ss->fail_high++;
                    if (legal == 0) {
                        ss->first_fail_high++;
                    }
                    return beta;
                }
                alpha = score;
                //best_move = move;
            }

            legal++;
        }
    }

    //if (alpha != old_alpha) {
    //    //printf("QUIE - %s - score %d\n", move_str(best_move), score);
    //    hash_put(&global_tp_table, b->hash_key, best_move, score, 0, b->ply, EXCA_FLAG);
    //}

    return alpha;
}
static int alpha_beta(S_BOARD *b, S_SEARCH_SETTINGS *ss, int alpha, int beta, int depth, int do_null) //, int window) //TODO: window?
{
    int i;
    int legal = 0;
    int old_alpha = alpha;
    int best_move = EMPTY;
    int score = -INFINITE;
    int best_score = -INFINITE;
    int null_score = -INFINITE;
    int move;
    int in_check;

    assert(debug_board(b));

    if (depth == 0) {
        return quiescence(b, ss, alpha, beta);
    }

    if (ss->nodes & 4095) {
        check_search_stop(ss);
    }

    ss->nodes++;

    if (b->search_ply && (is_repetition(b) || b->fifty_move_count >= 100)) {
        return 0;
    }

    if (b->search_ply >= MAX_PLY) {
        return eval_posistion(b);
    }

    S_HASHENTRY entry;

    if (probe_hash(&global_tp_table, b->hash_key, &entry, &score, alpha, beta, depth)) {
        global_tp_table.cut++;
        return score;
    }

    in_check = sq_attacked(b, b->piece_bb[KING_INDEX[b->side]], 1-b->side);

    if (in_check) {
        depth++;
    }

    if (do_null && b->search_ply && depth >= 4 && !in_check) {
        if (((b->side && WHITE_MAJ(b))) || ((!b->side) && BLACK_MAJ(b))) {
            make_null_move(b);
            b->search_ply++;

            null_score = -alpha_beta(b, ss, -beta, -beta+1, depth-4, false);

            unmake_null_move(b);
            b->search_ply--;

            if(null_score >= beta) {
                return beta;
            }
        }
    }

    S_MOVELIST l[1];
    generate_all_moves(b, l);

    if (hash_get(&global_tp_table, b->hash_key, &entry)) {
        for (i=0; i <l->index; i++) {
            if (l->moves[i].move == entry.move) {
                l->moves[i].score = 200000;
                break;
            }
        }
    }

    for (i = 0; i < l->index; i++) {

        set_best_move_next(i, l);
        move = l->moves[i].move;

        if (make_move(b, move)) {
            b->search_ply++;

            score = -alpha_beta(b, ss, -beta, -alpha, depth-1, do_null);

            unmake_move(b);
            b->search_ply--;

            if (ss->stop) {
                return 0;
            }

            if (score > best_score) {
                best_score = score;
                best_move = move;

                if (score > alpha) {
                    if (score >= beta) {
                        ss->fail_high++;
                        if (legal == 0) {
                            ss->first_fail_high++;
                        }

                        if(!mv_cap(move)) {
                            store_killer(b, move);
                        }

                        hash_put(&global_tp_table, b->hash_key, best_move, beta, depth, b->ply, BETA_FLAG);

                        return beta;
                    }

                    if(!mv_cap(move)) {
                        update_history(b, mv_piece(move), mv_to(move), b->ply);
                    }
                    alpha = score;
                }
            }

            legal++;
        }
    }

    if (legal == 0) {
        if (in_check) {
            return -MATE + b->search_ply;
        } else {
            return 0;
        }
    }

    if (alpha != old_alpha) {
        hash_put(&global_tp_table, b->hash_key, best_move, best_score, depth, b->ply, EXCA_FLAG);
    } else {
        hash_put(&global_tp_table, b->hash_key, best_move, alpha, depth, b->ply, ALPH_FLAG);
    }

    return alpha;
}

void search_position(S_BOARD *b, S_SEARCH_SETTINGS *ss)
{
    int best_moves[MAX_PLY];
    int best_move = EMPTY;
    int best_score = -INFINITE;
    int cur_depth = 0;
    int pv_moves = 0;
    int i;

    prepare_search(b, ss);

    for (cur_depth = 1; cur_depth <= ss->depth; cur_depth++) {
        best_score = alpha_beta(b, ss, -INFINITE, INFINITE, cur_depth, true);

        if (ss->stop) {
            break;
        }

        pv_moves = hash_get_pv_line(&global_tp_table, b, best_moves, cur_depth);
        printf("info score cp %d depth %d nodes %ld time %d", best_score, cur_depth, ss->nodes, cur_time_millis() - ss->starttime);

        best_move = best_moves[0];

        printf(" pv");
        for(i=0; i < pv_moves; i++) {
            printf(" %s", move_str(best_moves[i]));
        }
        printf("\n");
        //printf("Ordering: %.1f/%.1f = %.3f\n",ss->first_fail_high,ss->fail_high, ss->first_fail_high/ss->fail_high);
    }

    printf("bestmove %s\n", move_str(best_move));
}
