#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>
#include <semaphore.h>

#include "search.h"

#define JOB_BUF_SIZE 64

//semaphores for the job buffer
pthread_mutex_t start_lock;
pthread_mutex_t end_lock;
pthread_mutex_t start_search;
pthread_mutex_t report_move;
pthread_barrier_t barrier;

typedef struct {
    pthread_t *threads;
    //S_BOARD b;
    int size;
} S_THREADS;

typedef struct {
    S_BOARD b;
    volatile int _cur_job;
} S_JOB;

typedef struct {
    S_JOB buf[JOB_BUF_SIZE];
    int start;
    int end;
} S_JOB_BUFFER;

S_JOB_BUFFER jobs;

void thread_search_go();
void buffer_add_job(S_BOARD *b, int first_available_move);
void create_workers(S_THREADS *tt, int size, S_SEARCH_SETTINGS *ss);
void destroy_workers(S_THREADS *tt);
void init_search_barrier(unsigned int count);
void init_threads(int thread_count);
void signal_threads();

#endif /* THREADS_H */
