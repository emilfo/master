#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>
#include <semaphore.h>

#include "search.h"

typedef struct {
    pthread_t *threads;
    int size;
} S_THREADS;

void thread_search_go();
void create_workers(S_THREADS *tt, int size, S_SEARCH_SETTINGS *ss);
void destroy_workers(S_THREADS *tt);
void init_thread_cv();
void destroy_thread_cv();

#endif /* THREADS_H */
