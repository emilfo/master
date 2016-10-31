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
int threads_currently_searching;

volatile int g_search_id;

static void *thread_work_loop(void *th_id) 
{
    int thread_id = *(int *)th_id;

    while (true) {
        //wait until all threads have completed the current search before the
        //io-thread can setup a new search
        wait_search_complete_barrier();

        if (thread_id == 0) {
            g_depth = 0;
        }


        //wait until io has setup a new search
        wait_search_ready_barrier(); 

        g_thread_table.threads[thread_id].b.nodes = 0;

        if (g_search_info.quit) {
            break;
        }

        search_position(&g_thread_table.threads[thread_id].b, thread_id);
    }
    pthread_exit(NULL);
}

int get_search_id()
{
    if (g_thread_table.size == 1) return 1;

    volatile int tmp;
    volatile int search_id;
    do {
        tmp = g_search_id;
        search_id = (tmp%g_thread_table.size) + 1;
    } while(!(__sync_bool_compare_and_swap(&g_search_id, tmp, search_id)));

    search_id = (search_id % (g_thread_table.size/2)) + (g_thread_table.size/2);

    return search_id;
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

    //printf(">=== depth %d reached ===<\n", depth);

    g_depth = depth;

    return true;
}

void aquire_reportlock()
{
    pthread_spin_lock(&report_lock);
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

void start_threads()
{
    stop_threads();
    wait_search_ready_barrier();
    threads_currently_searching = true;
}

void stop_threads()
{
    if (threads_currently_searching) {
        g_search_info.stop = true;
        wait_search_complete_barrier();
        threads_currently_searching = false;
    }
}

static void create_workers(int size) 
{
    int i;
    //This shouldn't happen
    //if (g_thread_table.threads != NULL) {
    //    free(g_thread_table.threads);
    //}

    g_thread_table.size = size;
    g_thread_table.threads = (S_THREAD *)malloc(g_thread_table.size * sizeof(S_THREAD));

    for (i = 0; i < g_thread_table.size; i++) {
        g_thread_table.threads[i].thread_id = i;
        pthread_create(&g_thread_table.threads[i].thread, NULL, &thread_work_loop, (void *) &g_thread_table.threads[i].thread_id);
    }
}

static void destroy_workers() 
{
    int i;

    stop_threads();
    g_search_info.quit = true;
    start_threads();

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
    pthread_spin_init(&report_lock, g_depth);
    init_search_barriers(thread_count);
    create_workers(thread_count);
    threads_currently_searching = true;
    g_search_id = 0;
}

void destroy_threads()
{
    destroy_workers();
    destroy_search_barriers();
    pthread_spin_destroy(&report_lock);
}

void reinit_threads(int thread_count)
{
    destroy_threads();
    init_threads(thread_count);
    g_search_id = 0;
}
