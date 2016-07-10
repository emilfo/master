#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>
#include <semaphore.h>

#include "search.h"
#include "board.h"

#define JOB_BUF_SIZE 64

//semaphores for the job buffer
pthread_mutex_t main_thread;
pthread_barrier_t io_barrier;
pthread_barrier_t work_barrier;
pthread_barrier_t work_done_barrier;

typedef struct {
    pthread_t *threads;
    int size;
} S_THREADS;

typedef struct {
    S_BOARD *b;
    S_MOVELIST *l;

    //alphabeta values for threads
    int alpha;
    int beta;
    int depth;

    volatile int _cur_job;
} S_JOB;

S_JOB job;

void buffer_add_job(S_BOARD *b, S_MOVELIST *l, int start_move, int alpha, int beta, int depth);
void create_workers(S_THREADS *tt, int size, S_SEARCH_SETTINGS *ss);
void destroy_workers(S_THREADS *tt);
void init_search_barrier(unsigned int count);
void init_threads(int thread_count);
void io_signal_threads();
void work_signal_threads();
void work_loop();
void destroy_threads();

#endif /* THREADS_H */
