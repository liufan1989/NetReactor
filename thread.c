/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */

#include <sched.h>
#include <assert.h>
#include "thread.h"

pthread_key_t thread_key;

int bind_cpu_core(int cpu_id,pthread_t tid)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id,&mask);
    if (pthread_setaffinity_np(tid, sizeof(mask), &mask) != 0) {
           return -1;
    }
    return 0;
}

int create_thread(thread_t* ptd,pthread_func_t thread_func,void* args)
{
    assert(ptd != NULL);
    int             ret;
    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    if ((ret = pthread_create(&ptd->thread_id, &attr, thread_func, args)) != 0) {
        return -1;
    }
    return 0;
}


void wait_for_thread_startup(int nthreads)
{
    pthread_mutex_lock(&init_thread_startup_lock);
    while (init_thread_count < nthreads) {
        pthread_cond_wait(&init_thread_startup_cond, &init_thread_startup_lock);
    }
    pthread_mutex_unlock(&init_thread_startup_lock);
}


void thread_startup_over() {
    pthread_mutex_lock(&init_thread_startup_lock);
    init_thread_count++;
    pthread_cond_signal(&init_thread_startup_cond);
    pthread_mutex_unlock(&init_thread_startup_lock);
}






