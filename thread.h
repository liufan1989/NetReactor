
/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */

/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */

#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>

#include "log.h"
#include "event.h"

static int init_thread_count = 0;
static pthread_mutex_t init_thread_startup_lock =  PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t init_thread_startup_cond = PTHREAD_COND_INITIALIZER;

extern pthread_key_t thread_key;

typedef void *(*pthread_func_t)(void) ; 

typedef struct thread_s{
    pthread_t thread_id;
    int notify_recv_fd;    
    int notify_send_fd;
    conn_t conn;
    event_control_t* eventctr;
    log_t* logt;
}thread_t;

extern int bind_cpu_core(int cpuid,pthread_t tid);
extern int create_thread(thread_t* ptd,pthread_func_t thread_func,void* args);
extern void wait_for_thread_startup(int nthreads);
extern void thread_startup_over();


#endif



