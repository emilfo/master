#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "threads.h"
#include "search.h"
#include "globals.h"



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

static void buffer_remove_job() 
{
    /* spinlock to increment the ring-buffers start-location */
    while (!pthread_mutex_trylock(&start_lock));
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
void buffer_add_job(S_BOARD *b, S_MOVELIST *l, int start_index) {

    pthread_mutex_lock(&end_lock);
    while (!buffer_space_available());
    int new_end = (jobs.end+1)%JOB_BUF_SIZE;

    jobs.buf[new_end].b = b;
    jobs.buf[new_end].l = l;
    jobs.buf[new_end]._cur_job = start_index;
    jobs.end = new_end;

    pthread_mutex_unlock(&end_lock);
}

static void *new_thread_wait_for_work(void *search_settings) 
{
    int move_index;
    int job_index;
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
            pthread_mutex_unlock(&go_mutex);
        }
        while (go_search)

        job_index = jobs.start;
        while(job_index != jobs.end) { //jobs available
            move_index = __sync_fetch_and_add(&jobs.buf[job_index]._cur_job, 1);

            if (move_index == jobs.buf[job_index].l->index) {
                /* all jobs taken from this position, this thread gets the
                 * responsobility of sliding the ringbuffer count */
                buffer_remove_job();
                break;
            } else if (move_index > jobs.buf[job_index].l->index) {
                /* all jobs taken from this position, move on */
                break;
            }

            /* job to do */
            if(move_index == 0) {
                /* this only happens at the root node, as all other nodes have
                 * had their first move searched */
                search_position(&global_board, ss);

            } else {
                make_move_and_search(*jobs.buf[job_index].b, jobs.buf[job_index].l->moves[move_index].move, ss);
            }
            
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
        pthread_create(&tt->threads[i], NULL, &new_thread_wait_for_work, (void *) ss);
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

void init_buffer_semaphores() 
{
}

void destroy_buffer_semaphores() 
{
}
