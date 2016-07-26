#ifndef PERFT_H
#define PERFT_H

void perft_from_file(const char *filename, int divide);
void perft_fen(char *FEN, int divide, int depth);
int perft_divide(S_BOARD *b, int depth);
void eval_from_file(const char *filename);
void rating_from_file(const char *filename);
void bench_file(const char *filename);

#endif /* PERFT_H */
