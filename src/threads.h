#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>
#include <semaphore.h>

#include "search.h"

#define JOB_BUF_SIZE 64

//semaphores for the job buffer
pthread_mutex_t start_lock;
pthread_mutex_t end_lock;

typedef struct {
    pthread_t *threads;
    int size;
} S_THREADS;

typedef struct {
    S_BOARD *b;
    S_MOVELIST *l;
    volatile int _cur_job;
} S_JOBS;

typedef struct {
    S_JOBS buf[JOB_BUF_SIZE];
    int start;
    int end;
} S_JOB_BUFFER;

S_JOB_BUFFER jobs;

void thread_search_go();
void create_workers(S_THREADS *tt, int size, S_SEARCH_SETTINGS *ss);
void destroy_workers(S_THREADS *tt);
void init_thread_cv();
void destroy_thread_cv();

#endif /* THREADS_H */
