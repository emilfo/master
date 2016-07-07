#include <stdio.h>

#include "search.h"
#include "globals.h"
#include "eval.h"
#include "io.h"
#include "utils.h"
#include "movegen.h"
#include "board.h"
#include "uci.h"

#define BLACK_MAJ(b) (b->piece_bb[B_BISHOP] || b->piece_bb[B_ROOK] || b->piece_bb[B_QUEEN])
#define WHITE_MAJ(b) (b->piece_bb[W_BISHOP] || b->piece_bb[W_ROOK] || b->piece_bb[W_QUEEN])

static void report_score(S_BOARD *b, S_SEARCH_SETTINGS *ss, int score, int move_index);

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
    int best_move = EMPTY;
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
                best_move = move;
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

//static void thread_alpha_beta_slave(S_BOARD *b, S_SEARCH_SETTINGS *ss, int do_null) //, int window) //TODO: window?
//{
//    if (depth <= 2) {
//        score = -alpha_beta(b, ss, -b->beta, -b->alpha, b->depth-1, false);
//        report_score(b, score);
//    }
//}

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
    int hash_hit;
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

    hash_hit = probe_hash(&global_tp_table, b->hash_key, &entry, &score, alpha, beta, depth);
    if (hash_hit) {
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

    if (hash_hit) {
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

static void save_and_report_up(S_BOARD *b, S_SEARCH_SETTINGS *ss, int score, int move)
{
    if (score > b->alpha) {
        if (score >= b->beta) {
            //TODO hash store beta
        }
        //TODO hash store exact
        hash_put(&global_tp_table, b->hash_key, move, score, ss->cur_depth, b->ply, EXCA_FLAG);
    } else {
        //TODO hash store alpha
    }

    if (b->prev_board == NULL) {
        uci_report_scores(b, ss, score);
        pthread_mutex_unlock(&start_search);
    } else {
        report_score(b->prev_board, ss, -score, b->prev_move_index);
    }
}

static void report_score(S_BOARD *b, S_SEARCH_SETTINGS *ss, int score, int move_index)
{
    int i;
    int best_score = -INFINITE;
    int best_move_index = 0;
    int move_cut_beta;


    if (b->beta_cut) { //TODO: check boards backwards
        /* Already has a beta cut on this branch */
        return;
    }

    if (score >= b->beta) {
        move_cut_beta = __sync_bool_compare_and_swap (&b->beta_cut, 0, 1);

        if (move_cut_beta) {
            /* This move cut beta, and can report upwards immediately*/
            save_and_report_up(b->prev_board, ss, score, b->prev_move_index);
            return;
        }
    }

    if (b->depth_left <= 2) {
        /* this thread search all moves alone */
        save_and_report_up(b, ss, score, move_index);
        return;
    }

    b->l.moves[move_index].eval = score;

    int report_count = __sync_fetch_and_add(&b->l.returned, 1);

    if (report_count == b->l.index-1) {
        /* this is the last thread to report it's score it therefore gets the
         * responibility to report the score up one more level */

        for (i = 0; i < b->l.index; i++) {
            if (b->l.moves[i].eval > best_score) {
                best_score = b->l.moves[i].eval;
                best_move_index = i;
            }

        }

        save_and_report_up(b, ss, best_score, best_move_index);
    }
}

static void thread_alpha_beta(S_BOARD *b, S_SEARCH_SETTINGS *ss) //, int window) //TODO: window?
{
    int i;
    int legal;
    int score;
    int in_check;
    int move = EMPTY;

    if (b->depth_left <= 2) {
        score = -alpha_beta(b, ss, -b->beta, -b->alpha, b->depth_left-1, false);
        report_score(b->prev_board, ss, score, b->prev_move_index);
        return;
    }

    in_check = sq_attacked(b, b->piece_bb[KING_INDEX[b->side]], 1-b->side);

    if (in_check) {
        //depth++; TODO search extension
    }

    generate_all_moves(b, &b->l);

    i = 0;
    legal = false;
    do {
        set_best_move_next(i, &b->l);
        move = b->l.moves[i++].move;
        legal = make_move(b, move);
    } while (!legal);

    if (!legal) {
        if (legal == 0) {
            if (in_check) {
                report_score(b->prev_board, ss, -MATE + b->search_ply, b->prev_move_index);
            } else {
                report_score(b, ss, 0, b->prev_move_index);
            }
            return;
        }
    }

    unmake_move(b);

    buffer_add_job(b, i);
}

/**
 * younger brothers gets a board and a move they have to do before searching.
 * the board is not sent by reference, so here they have a local copy of the
 * board struct
 */
void make_move_and_search(S_BOARD b, S_BOARD *prev_board, int move_index, S_SEARCH_SETTINGS *ss)
{
    //TODO check if prev beta_cut before making move
    if (make_move(&b, b.l.moves[move_index].move)) {
        b.prev_board = prev_board;

        b.beta_cut = 0;
        //reverse alpha and beta-values in negamax fashion
        b.alpha = -prev_board->beta;
        b.beta = -prev_board->alpha;
        b.prev_move_index = move_index;
        b.depth_left--;

        thread_alpha_beta(&b, ss);
    }
}

void start_new_search(S_BOARD *b, S_SEARCH_SETTINGS *ss)
{
    ss->cur_depth++;

    b->beta_cut = 0;
    b->alpha = -INFINITE;
    b->beta = INFINITE;
    b->depth_left = ss->cur_depth;
    b->prev_board = NULL;

    if (ss->cur_depth > ss->depth) {
        ss->stop = true;
        return;
    }

    thread_alpha_beta(b, ss);
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
