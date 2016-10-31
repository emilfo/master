#ifndef DEBUG_H
#define DEBUG_H

int valid_sq(const int sq);
int valid_side(const int side);
int valid_piece(const int pce);
int valid_piece_or_empty(const int pce);
int valid_bool(const int pce);
int print_depth(int thread_id, int cur_depth);

#endif /* DEBUG_H */
