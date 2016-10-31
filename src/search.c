#include <stdio.h>
#include <inttypes.h>

#include "search.h"
#include "globals.h"
#include "defs.h"
#include "hashtable.h"
#include "eval.h"
#include "io.h"
#include "utils.h"
#include "movegen.h"
#include "board.h"

#define BLACK_MAJ(b) (b->piece_bb[B_BISHOP] || b->piece_bb[B_ROOK] || b->piece_bb[B_QUEEN])
#define WHITE_MAJ(b) (b->piece_bb[W_BISHOP] || b->piece_bb[W_ROOK] || b->piece_bb[W_QUEEN])

S_SEARCH_SETTINGS g_search_info;
u32 g_best_move;

volatile int g_depth;

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

static void check_search_stop()
{
    if (g_search_info.time_set && cur_time_millis() >= g_search_info.stoptime) {
        g_search_info.stop = true;
    }
}

static void thread_setup_board(S_BOARD *b)
{
    int i, j;

    for (i = 0; i < 13; i++) {
        b->piece_bb[i] = g_board.piece_bb[i];
    }
    for (i = 0; i < 3; i++) {
        b->all_piece_bb[i] = g_board.all_piece_bb[i];
    }

    b->side = g_board.side;
    b->castle_perm = g_board.castle_perm;
    b->ep_sq = g_board.ep_sq;
    b->fifty_move_count = g_board.fifty_move_count;

    for (i = 0; i < 64; i++) {
        b->sq[i] = g_board.sq[i];
    }

    b->hash_key = g_board.hash_key;

    for (i = 0; i < g_board.ply; i++) {
        b->prev[i] = g_board.prev[i];
    }

    b->ply = g_board.ply;
    b->search_ply = 0;

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

    b->nodes = 0;
    b->fail_high = 0;
    b->first_fail_high = 0;
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

static i16 quiescence(S_BOARD *b, int alpha, int beta)
{
    int i;
    int legal = 0;
    //int old_alpha = alpha;
    //int best_move = EMPTY;
    i16 score;
    u32 move;

    assert(debug_board(b));

    if ((b->nodes & 4095) == 0) {
        check_search_stop();
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

            score = -quiescence(b, -beta, -alpha);

            unmake_move(b);
            b->search_ply--;

            if (g_search_info.stop) {
                return 0;
            }

            if (score > alpha) {
                if (score >= beta) {
                    b->fail_high++;
                    if (legal == 0) {
                        b->first_fail_high++;
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
static i16 alpha_beta(S_BOARD *b, int alpha, int beta, int depth, int do_null, int search_depth) //, int window) //TODO: window?
{
    int i;
    int legal = 0;
    int in_check;
    i16 old_alpha = alpha;
    i16 score = -INFINITE;
    i16 best_score = -INFINITE;
    i16 null_score = -INFINITE;
    u32 move;
    u32 best_move = EMPTY;

    assert(debug_board(b));

    if (depth == 0) {
        return quiescence(b, alpha, beta);
    }

    if ((b->nodes & 4095) == 0) {
        check_search_stop();
    }

    b->nodes++;

    if (b->search_ply && (is_repetition(b) || b->fifty_move_count >= 100)) {
        return 0;
    }

    if (b->search_ply >= MAX_PLY) {
        return eval_posistion(b);
    }

    S_HASHENTRY entry;

    if (probe_hash(b->hash_key, &entry, &score, alpha, beta, depth)) {
        g_hash_table.cut++;
        if (score >= alpha) {
            b->principal_variation[b->search_ply] = entry.move;
        }
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

            null_score = -alpha_beta(b, -beta, -beta+1, depth-4, false, search_depth);

            unmake_null_move(b);
            b->search_ply--;

            if(null_score >= beta) {
                return beta;
            }
        }
    }

    S_MOVELIST l[1];
    generate_all_moves(b, l);

    if (hash_get(b->hash_key, &entry)) {
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

            score = -alpha_beta(b, -beta, -alpha, depth-1, do_null, search_depth);

            unmake_move(b);
            b->search_ply--;

            if (g_search_info.stop || g_depth >= search_depth) {
                return 0;
            }

            if (score > best_score) {
                best_score = score;
                best_move = move;

                if (score > alpha) {
                    if (score >= beta) {
                        b->fail_high++;
                        if (legal == 0) {
                            b->first_fail_high++;
                        }

                        if(!mv_cap(move)) {
                            store_killer(b, move);
                        }

                        hash_put(b->hash_key, best_move, beta, depth, b->ply, BETA_FLAG);

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
        hash_put(b->hash_key, best_move, best_score, depth, b->ply, EXCA_FLAG);
        b->principal_variation[b->search_ply] = best_move;
    } else {
        hash_put(b->hash_key, best_move, alpha, depth, b->ply, ALPH_FLAG);
    }

    return alpha;
}

long count_all_nodes()
{
    int i;
    long all_nodes = 0L;
    for (i = 0; i < g_thread_table.size; i++) {
        all_nodes += g_thread_table.threads[i].b.nodes;
    }

    return all_nodes;
}

#ifdef DDEBUG
static int print_depth(int thread_id, int cur_depth)
{
    if (thread_id == 0)
        printf("%2d|  |  |  |  |  |  |  \n",cur_depth);
    if (thread_id == 1)
        printf("  |%2d|  |  |  |  |  |  \n",cur_depth);
    if (thread_id == 2)
        printf("  |  |%2d|  |  |  |  |  \n",cur_depth);
    if (thread_id == 3)
        printf("  |  |  |%2d|  |  |  |  \n",cur_depth);
    if (thread_id == 4)
        printf("  |  |  |  |%2d|  |  |  \n",cur_depth);
    if (thread_id == 5)
        printf("  |  |  |  |  |%2d|  |  \n",cur_depth);
    if (thread_id == 6)
        printf("  |  |  |  |  |  |%2d|  \n",cur_depth);
    if (thread_id == 7)
        printf("  |  |  |  |  |  |  |%2d\n",cur_depth);

    return 1;
}
#endif

void search_position(S_BOARD *b, int thread_id)
{
    i16 best_score;
    u32 best_moves[MAX_PLY];
    int pv_move_count;
    int cur_depth;
    int i;
    int a_index;
    int b_index;
    volatile i16 alpha;
    volatile i16 beta;
    long total_nodes;

    thread_setup_board(b);
    int first = true;

    //temporary best_score for aspiration window
    best_score = quiescence(b, -INFINITE, INFINITE);

    while (g_depth < g_search_info.depth) {

        //TODO: Here you can change vals for testing
        //cur_depth = g_depth + 1 + (thread_id%2); 

        if (first) {
            cur_depth = g_depth + 1 + (__builtin_ctz(thread_id+1));
	    first = false;
	} else {
            cur_depth = g_depth + 1 + (__builtin_ctz(get_search_id()));
	}

        assert(print_depth(thread_id, cur_depth));

        //Alpha and beta are set to the aspiration window from previous search
        alpha = MAX(-INFINITE, (best_score - aspiration_window[0]));
        beta  = MIN(INFINITE, (best_score + aspiration_window[0]));
        a_index = b_index = 1;


        do {
            //printf("a:%d b:%d\n", alpha, beta);
            best_score = alpha_beta(b, alpha, beta, cur_depth, true, cur_depth);
            if (g_search_info.stop) {
                break;
            }

            if (best_score <= alpha) { //fail low
                //printf("fail low\n");
                alpha = MAX(-INFINITE, best_score - aspiration_window[MIN(a_index, 3)]);
                a_index++;
            } else if (best_score >= beta) { //fail high
                //printf("fail high\n");
                beta  = MIN(INFINITE, (best_score + aspiration_window[MIN(b_index,3)]));
                b_index++;
            } else {
                break;
            }
        } while(true);

        if (g_search_info.stop) {
            break;
        }

        //only report the result if the deepest yet
        //printf("aquiring report lock\n");
        if (aquire_reportlock_if_deepest(cur_depth)) {
            if (cur_depth >= g_search_info.depth) {
                g_search_info.stop = true;
            }

            total_nodes = count_all_nodes();

            printf("thread %d ", thread_id);
            pv_move_count = hash_get_pv_line(b, best_moves, cur_depth);
            printf("info score cp %d depth %d nodes %ld time %"PRIu64"", best_score, cur_depth, total_nodes, cur_time_millis() - g_search_info.starttime);

            g_best_move = best_moves[0];

            printf(" pv");
            for(i=0; i < pv_move_count; i++) {
                printf(" %s", move_str(best_moves[i]));
            }
            printf("\n");

            release_reportlock();
        }

    }

    //only one thread prints best move
    if (thread_id == 0) {
        aquire_reportlock();
        printf("bestmove %s\n", move_str(g_best_move));
        release_reportlock();
    }
}
