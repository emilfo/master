#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>
#include <semaphore.h>

#include "board.h"

typedef struct {
    pthread_t thread;
    S_BOARD b;
    int thread_id;
} S_THREAD;

typedef struct {
    S_THREAD *threads;
    int size;
} S_THREADS;

/* Report_lock is used to ensure that only one thread reports its result to the
 * UI at a time */
int aquire_reportlock_if_deepest(int depth);
void aquire_reportlock();
void release_reportlock();

/* Barriers used to ensure that the all workers have completed searching, before
 * the main/io-thread can setup a new search. And that the io thread is done
 * setting up a new search before workers can start a new search */
void wait_search_complete_barrier();
void wait_search_ready_barrier();
void start_threads();
void stop_threads();

int get_search_id();

/* init and destroy used when starting up and closing the program, reinit is
 * used if the UI asks to change the number of threads */
void init_threads(int thread_count);
void destroy_threads();
void reinit_threads(int thread_count);

extern int threads_currently_searching;

#endif /* THREADS_H */
