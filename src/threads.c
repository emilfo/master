#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "threads.h"
#include "search.h"
#include "globals.h"
#include "defs.h"

static pthread_spinlock_t report_lock;
static pthread_barrier_t search_complete_barrier;
static pthread_barrier_t search_ready_barrier;

S_THREADS g_thread_table;


static void *thread_work_loop(void *th_id) 
{
    int thread_id = *(int *)th_id;

    while (true) {
        //wait until all threads have completed the current search before the
        //io-thread can setup a new search
        wait_search_complete_barrier();

        g_thread_table.threads[thread_id].b.nodes = 0;

        //wait until io has setup a new search
        wait_search_ready_barrier(); 

        if (g_search_info.quit) {
            break;
        }

        search_position(&g_thread_table.threads[thread_id].b, thread_id);
    }
    pthread_exit(NULL);
}

int aquire_reportlock_if_deepest(int depth)
{
    do {
        if (g_depth >= depth) {
            return false;
        }
    } while(pthread_spin_trylock(&report_lock) != 0);

    if (g_depth >= depth) {
        pthread_spin_unlock(&report_lock);
        return false;
    }

    g_depth = depth;

    return true;
}

void release_reportlock()
{
    pthread_spin_unlock(&report_lock);
}

void wait_search_complete_barrier()
{
    pthread_barrier_wait(&search_complete_barrier);
}

void wait_search_ready_barrier()
{
    pthread_barrier_wait(&search_ready_barrier);
}

static void create_workers(int size) 
{
    int i;
    if (g_thread_table.threads != NULL) {
        free(g_thread_table.threads);
    }

    g_thread_table.size = size;
    g_thread_table.threads = (S_THREAD *)malloc(g_thread_table.size * sizeof(S_THREAD));

    for (i = 0; i < g_thread_table.size; i++) {
        pthread_create(&g_thread_table.threads[i].thread, NULL, &thread_work_loop, (void *) &i);
    }
}

static void destroy_workers() 
{
    int i;

    g_search_info.quit = true;
    g_search_info.stop = true;


    //TODO what barriers should we wait for here?

    for(i = 0; i < g_thread_table.size; i++) {
        pthread_join(g_thread_table.threads[i].thread, NULL);
    }

    g_thread_table.size = 0;
    if (g_thread_table.threads != NULL) {
        free(g_thread_table.threads);
    }
}

static void init_search_barriers(int thread_count)
{
    pthread_barrier_init(&search_ready_barrier, NULL, thread_count+1);
    pthread_barrier_init(&search_complete_barrier, NULL, thread_count+1);
}


static void destroy_search_barriers()
{
    pthread_barrier_destroy(&search_ready_barrier);
    pthread_barrier_destroy(&search_complete_barrier);
}

void init_threads(int thread_count)
{
    init_search_barriers(thread_count);
    create_workers(thread_count);
}

void destroy_threads()
{
    destroy_workers();
    destroy_search_barriers();
}

void reinit_threads(int thread_count)
{
    destroy_threads();
    init_threads(thread_count);
}
