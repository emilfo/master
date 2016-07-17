#include <stdio.h>

#include "search.h"
#include "globals.h"
#include "eval.h"
#include "io.h"
#include "utils.h"
#include "movegen.h"
#include "board.h"
#include "uci.h"
#include "threads.h"

#define BLACK_MAJ(b) (b->piece_bb[B_BISHOP] || b->piece_bb[B_ROOK] || b->piece_bb[B_QUEEN])
#define WHITE_MAJ(b) (b->piece_bb[W_BISHOP] || b->piece_bb[W_ROOK] || b->piece_bb[W_QUEEN])
#define MIN(a, b) (( a < b)? a : b )
#define MAX(a, b) (( a > b)? a : b )

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
            global_search_history[i][j] = 0;
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < MAX_PLY; j++) {
            global_search_killers[i][j] = 0;
        }
    }

    b->search_ply = 0;

}

static void store_killer(S_BOARD *b, int move) 
{
    global_search_killers[1][b->search_ply] = global_search_killers[0][b->search_ply];
    global_search_killers[0][b->search_ply] = move;
}

static void update_history(S_BOARD *b, int piece, int to_sq, int depth)
{
    global_search_history[piece][to_sq] += depth;
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

    if ((b->nodes & 4095) == 0) {
        check_search_stop(ss);
    }

    b->nodes++;

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
    //printf("alpha beta, depth:%d\n", depth);
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

    if ((b->nodes & 4095) == 0) {
        check_search_stop(ss);
    }

    b->nodes++;


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

static int split_alpha_beta(S_BOARD *b, S_SEARCH_SETTINGS *ss, int alpha, int beta, int depth, S_BOARD *my_b)
{
    //printf("split alphabeta\n");
    int i;
    int best_score = -INFINITE;
    int best_move = EMPTY;
    int score = -INFINITE;
    int old_alpha = alpha;
    S_HASHENTRY entry;
    int move;
    int legal = 0;
    int in_check = false;
    b->nodes = 0L;

    
    if (probe_hash(&global_tp_table, b->hash_key, &entry, &score, alpha, beta, depth)) {
        global_tp_table.cut++;
        return score;
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
        //printf("i:%d\n", i);

        set_best_move_next(i, l);
        move = l->moves[i].move;

        if (make_move(b, move)) {
            b->search_ply++;

            //printf("before alpha beta\n");
            score = -alpha_beta(b, ss, -beta, -alpha, depth-1, false);
            //printf("after alpha beta, score:%d\n", score);
            //printf("move:%s, score:%d\n", move_str(move), score);
            l->moves[i].eval = score;
            l->moves[i].nodes = b->nodes;

            unmake_move(b);
            b->search_ply--;

            if (ss->stop) {
                return 0;
            }

            if (score > best_score) {
                //printf("yo\n");
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
                        //printf("yo2, beta:%d\n", beta);

                        hash_put(&global_tp_table, b->hash_key, best_move, beta, depth, b->ply, BETA_FLAG);

                        ss->nodes += b->nodes;
                        return beta;
                    }

                    if(!mv_cap(move)) {
                        update_history(b, mv_piece(move), mv_to(move), b->ply);
                    }
                    alpha = score;
                }
            }

            //printf("yo3\n");
            legal++;
            break;
        } else {
            l->moves[i].eval = -INFINITE;
            l->moves[i].nodes = 0L;
        }
            
    }

    if (legal == 0) {
        if (in_check) {
            return -MATE + b->search_ply;
        } else {
            return 0;
        }
    }

    //printf("adding job\n");
    buffer_add_job(b, l, i, alpha, beta, depth);

    //printf("signal1\n");
    work_signal_threads();
    //To avoid concurrency issues we make sure all threads have been notified
    //before anyone begin searching, therefore we have to signal twice
    //printf("signal2\n");
    work_signal_threads(); 
    //printf("work-loop\n");
    work_loop(ss, my_b);

    for (i = 0; i < l->index; i++) {
        score = l->moves[i].eval;
        ss->nodes += l->moves[i].nodes;
        if (score > best_score) {
            best_score = score;
            best_move = l->moves[i].move;
        }
    }

    if (best_score > alpha) {
        if (best_score >= beta) {

            if(!mv_cap(best_move)) {
                store_killer(b, best_move);
            }

            hash_put(&global_tp_table, b->hash_key, best_move, beta, depth, b->ply, BETA_FLAG);

            return beta;
        }

        if(!mv_cap(best_move)) {
            update_history(b, mv_piece(best_move), mv_to(best_move), b->ply);
        }
        alpha = best_score;
    }

    if (alpha != old_alpha) {
        hash_put(&global_tp_table, b->hash_key, best_move, best_score, depth, b->ply, EXCA_FLAG);
    } else {
        hash_put(&global_tp_table, b->hash_key, best_move, alpha, depth, b->ply, ALPH_FLAG);
    }
    return alpha;
}

static int thread_alpha_beta(S_BOARD *b, S_SEARCH_SETTINGS *ss, int alpha, int beta, int depth, int do_null, S_BOARD *my_b)
{
    int score = -INFINITE;

    if (depth == 4) {
        score = split_alpha_beta(b, ss, alpha, beta, depth, my_b);
        //printf ("returning score:%d\n", score);
        return score;
    }

    int i;
    int best_score = -INFINITE;
    int best_move = EMPTY;
    int old_alpha = alpha;
    int legal = 0;
    int null_score = -INFINITE;
    int move;
    int in_check;

    if (depth == 0) {
        return quiescence(b, ss, alpha, beta);
    }

    if ((ss->nodes & 4095) == 0) {
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

    if (do_null && b->search_ply && depth > 4 && !in_check) {
        if (((b->side && WHITE_MAJ(b))) || ((!b->side) && BLACK_MAJ(b))) {
            make_null_move(b);
            b->search_ply++;

            null_score = -thread_alpha_beta(b, ss, -beta, -beta+1, depth-4, false, my_b);

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

            score = -thread_alpha_beta(b, ss, -beta, -alpha, depth-1, do_null, my_b);
            //printf("move:%s, score:%d\n", move_str(move), score);

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

void make_move_and_search(S_BOARD *b, S_SEARCH_SETTINGS *ss, S_MOVELIST *l, int move_index, int alpha, int beta, int depth, S_BOARD *my_b)
{
    int i;
    
    for (i = 0; i < 13; i++) my_b->piece_bb[i] = b->piece_bb[i];
    for (i = 0; i < 3; i++) my_b->all_piece_bb[i] = b->all_piece_bb[i];
    my_b->side = b->side; my_b->castle_perm = b->castle_perm;
    my_b->ep_sq = b->ep_sq; my_b->fifty_move_count = b->fifty_move_count;
    for (i = 0; i < 64; i++) my_b->sq[i] = b->sq[i];
    my_b->ply = b->ply; my_b->search_ply = b->search_ply;
    for (i = 0; i < b->ply; i++) my_b->prev[i] = b->prev[i];
    
    if (make_move(my_b, l->moves[move_index].move)) {
        int score = -alpha_beta(my_b, ss, -beta, -alpha, depth-1, false);
        l->moves[move_index].eval = score;
        l->moves[move_index].nodes = my_b->nodes;
    } else {
        l->moves[move_index].eval = -INFINITE;
        l->moves[move_index].nodes = 0L;
    }
}

void search_position(S_BOARD *b, S_SEARCH_SETTINGS *ss, S_BOARD *my_b)
{
    int best_moves[MAX_PLY];
    int best_move = EMPTY;
    int best_score = -INFINITE;
    int cur_depth = 0;
    int pv_moves = 0;
    int i;
    int a_index;
    int b_index;
    volatile int alpha = -INFINITE;
    volatile int beta = INFINITE;

    prepare_search(b, ss);

    for (cur_depth = 1; cur_depth <= ss->depth; cur_depth++) {
        //printf("searching depth %d\n", cur_depth);
        do {
            best_score = thread_alpha_beta(b, ss, alpha, beta, cur_depth, true, my_b);
            if (ss->stop) {
                break;
            }

            if (best_score <= alpha) { //fail low
                alpha = MAX(-INFINITE, best_score - aspiration_window[MIN(a_index, 3)]);
                a_index++;
            } else if (best_score >= beta) { //fail high
                beta  = MIN(INFINITE, (best_score + aspiration_window[MIN(b_index,3)]));
                b_index++;
            } else {
                break;
            }
        } while(true);

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

        alpha = MAX(-INFINITE, (best_score - aspiration_window[0]));
        beta  = MIN(INFINITE, (best_score + aspiration_window[0]));
        a_index = b_index = 1;
    }

    printf("bestmove %s\n", move_str(best_move));
}
