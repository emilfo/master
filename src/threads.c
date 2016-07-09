#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "threads.h"
#include "movegen.h"
#include "search.h"
#include "globals.h"
#include "io.h"
#include "uci.h"

static void wait_for_io_signal()
{
    pthread_barrier_wait(&io_barrier);
}

void io_signal_threads()
{
    pthread_barrier_wait(&io_barrier);
}

static void wait_for_work_signal()
{
    pthread_barrier_wait(&work_barrier);
}

void work_signal_threads()
{
    pthread_barrier_wait(&work_barrier);
}

void buffer_add_job(S_BOARD *b, S_MOVELIST *l, int alpha, int beta, int depth)
{
    job.b = b;
    job.l = l;
    job.alpha = alpha;
    job.beta = beta;
    job.depth = depth;
    job._cur_job = 0;
}

static int get_job(int *move_index)
{
   *move_index = __sync_fetch_and_add(&job._cur_job, 1);

   if (*move_index < job.l->index) {
       return true;
   }
   return false;
}

void work_loop(S_SEARCH_SETTINGS *ss)
{
    int move_index;

    while (true) {
        if (ss->stop) {
            break;
        }


        if (!get_job(&move_index)) {
            //printf("out of jobs\n");
            break;
        }
        make_move_and_search(*job.b, ss, job.l, move_index, job.alpha, job.beta, job.depth);

    }
}

static void thread_wait_for_work(S_SEARCH_SETTINGS *ss) 
{
    while (true) {
        //printf("waiting for work\n");
        wait_for_work_signal();
        if(ss->stop) {
            break;
        }
        work_loop(ss);
    }
}

static void *thread_wait_for_io() 
{
    while (true) {
        wait_for_io_signal();

        if (global_search_settings.quit) {
            break; 
        }

        if (pthread_mutex_trylock(&main_thread) == 0) {
            /* One thread is main, which allocates work to the others */
            printf("main-thread\n");
            search_position(&global_board, &global_search_settings);
            pthread_mutex_unlock(&main_thread);
        } else {
            printf("workerthread\n");
            thread_wait_for_work(&global_search_settings);
        }
    }

    pthread_exit(NULL);
}

void create_workers(S_THREADS *tt, int size, S_SEARCH_SETTINGS *ss) 
{
    int i;

    tt->size = size;
    if (tt->threads != NULL) {
        destroy_workers(tt);
    }

    tt->threads = (pthread_t *)malloc(tt->size * sizeof(pthread_t));

    for (i = 0; i < tt->size; i++) {
        pthread_create(&tt->threads[i], NULL, &thread_wait_for_io, NULL);
    }
}

void destroy_workers(S_THREADS *tt) 
{
    int i;

    global_search_settings.quit = true;
    global_search_settings.stop = true;


    work_signal_threads();
    io_signal_threads();

    for(i = 0; i < tt->size; i++) {
        pthread_join(tt->threads[i], NULL);
    }

    tt->size = 0;
    if (tt->threads != NULL) {
        free(tt->threads);
    }
}

void init_threads(int thread_count)
{
    pthread_mutex_init(&main_thread, NULL);
    pthread_barrier_init(&io_barrier, NULL, thread_count+1);
    pthread_barrier_init(&work_barrier, NULL, thread_count);
    create_workers(&global_thread_table, thread_count, &global_search_settings);
}

void destroy_threads()
{
    destroy_workers(&global_thread_table);
    pthread_barrier_destroy(&io_barrier);
    pthread_barrier_destroy(&work_barrier);
    pthread_mutex_destroy(&main_thread);
}
