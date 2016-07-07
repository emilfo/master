#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "threads.h"
#include "movegen.h"
#include "search.h"
#include "globals.h"
#include "io.h"
#include "uci.h"

static void wait_for_signal()
{
    pthread_barrier_wait(&barrier);
}

void signal_threads()
{
    pthread_barrier_wait(&barrier);
}

static void buffer_remove_job() 
{
    /* spinlock to increment the ring-buffers start-location */
    while (pthread_mutex_trylock(&start_lock) != 0);
    jobs.start = (jobs.start+1)%JOB_BUF_SIZE;
    pthread_mutex_unlock(&start_lock);
}

static int buffer_space_available() 
{
    if (jobs.end < jobs.start) {
        return (JOB_BUF_SIZE-jobs.start + jobs.end > 2);
    } else {
        return (jobs.end - jobs.start > 2);
    }
}

void buffer_add_job(S_BOARD *b, int first_available_move)
{

    pthread_mutex_lock(&end_lock);
    while (!buffer_space_available()); //spinlock on buffer add_space

    int new_end = (jobs.end+1)%JOB_BUF_SIZE;

    jobs.buf[new_end].b = *b;
    jobs.buf[new_end].b = *b;
    jobs.buf[new_end]._cur_job = first_available_move;
    jobs.end = new_end;

    pthread_mutex_unlock(&end_lock);
}

static void thread_work_loop(S_SEARCH_SETTINGS *ss)
{
    S_JOB *job;
    S_MOVELIST *l;
    int job_index, move_index;

    while (true) {
        job_index = jobs.start;

        //spinlock wait for jobs
        while (job_index == jobs.end) {
            if (ss->stop) {
                break;
            }

            if (pthread_mutex_trylock(&start_search) == 0) {
                /* One thread starts search at new depth */
                start_new_search(&global_board, ss);
            }
        }

        if (ss->stop) {
            break;
        }

        job = &jobs.buf[job_index];
        l = &job->b.l;

        /* atomic increment assures that each thread gets a different move */
        move_index = __sync_fetch_and_add(&job->_cur_job, 1);

        if (move_index >= l->index) {
            /* All moves are taken from this job */
            if (move_index == l->index) {
                buffer_remove_job();
            }

            continue;
        }
        
        make_move_and_search(job->b, &job->b, l->moves[move_index].move, ss);

    }

    if (pthread_mutex_trylock(&report_move)) {
        uci_print_bestmove();
    }
}

static void *thread_wait_for_work(void *search_settings) 
{
    S_SEARCH_SETTINGS *ss = (S_SEARCH_SETTINGS *)search_settings;

    while (true) {
        wait_for_signal();

        if (ss->quit) {
            break; 
        }
        
        thread_work_loop(ss);
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
        pthread_create(&tt->threads[i], NULL, &thread_wait_for_work, (void *) ss);
    }
}

void destroy_workers(S_THREADS *tt) 
{
    int i;

    global_search_settings.quit = true;
    signal_threads();

    for(i = 0; i < tt->size; i++) {
        pthread_join(tt->threads[i], NULL);
    }

    tt->size = 0;
    if (tt->threads != NULL) {
        free(tt->threads);
    }
}

static void init_job_buffer()
{
    jobs.start = 0;
    jobs.end = 0;
}

//void destroy_job_buffer()
//{
//}

void init_threads(int thread_count)
{
    init_job_buffer();
    pthread_mutex_init(&report_move, NULL);
    pthread_mutex_init(&start_lock, NULL);
    pthread_mutex_init(&end_lock, NULL);
    pthread_barrier_init(&barrier, NULL, thread_count+1);
    create_workers(&global_thread_table, thread_count, &global_search_settings);
}

void destroy_threads()
{
    destroy_workers(&global_thread_table);
    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&end_lock);
    pthread_mutex_destroy(&start_lock);
}
