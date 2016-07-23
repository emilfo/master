#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "threads.h"
#include "search.h"
#include "globals.h"
#include "defs.h"

pthread_mutex_t go_mutex;
pthread_cond_t go_cv;
int go_search;
int debug_print;


static void *thread_wait_for_work(void *search_settings) 
{
    S_SEARCH_SETTINGS *ss = (S_SEARCH_SETTINGS *)search_settings;

    while (true) {
        pthread_mutex_lock(&go_mutex);
        while (!go_search) {
            pthread_cond_wait(&go_cv, &go_mutex);
        }

        if (ss->quit) {
            pthread_mutex_unlock(&go_mutex);
            break;
        } else {
            go_search = false;
            pthread_mutex_unlock(&go_mutex);

            search_position(&global_board, ss);
        }
    }
    pthread_exit(NULL);
}

void thread_search_go() 
{
    pthread_mutex_lock(&go_mutex);
    go_search = true;
    pthread_cond_signal(&go_cv);
    pthread_mutex_unlock(&go_mutex);
}

void create_workers(S_THREADS *tt, int size, S_SEARCH_SETTINGS *ss) 
{
    int i;
    tt->size = size;
    if (tt->threads != NULL) {
        free(tt->threads);
    }

    tt->threads = (pthread_t *)malloc(tt->size * sizeof(pthread_t));

    for (i = 0; i < tt->size; i++) {
        pthread_create(&tt->threads[i], NULL, &thread_wait_for_work, (void *) ss);
    }
}

void destroy_workers(S_THREADS *tt) 
{
    int i;
    for(i = 0; i < tt->size; i++) {
        pthread_join(tt->threads[i], NULL);
    }

    tt->size = 0;
    if (tt->threads != NULL) {
        free(tt->threads);
    }
}

void init_thread_cv() 
{
    pthread_mutex_init(&go_mutex, NULL);
    pthread_cond_init (&go_cv, NULL);
}
void destroy_thread_cv() 
{
    pthread_mutex_destroy(&go_mutex);
    pthread_cond_destroy(&go_cv);
}
