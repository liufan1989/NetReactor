
/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */
#ifndef __EVENT_H__
#define __EVENT_H__

#include <pthread.h>

#include "uthash.h"

#ifdef LF_DEBUG
#define INLINE 
#else
#define INLINE inline
#endif

#define EVENT_THRESHOLD_COUNT           128
#define EVENT_INIT_COUNT                1024
#define EVENT_DELTA_COUNT               1024 
#define EVENT_DELTA_MEM_ALLOC_MAX_COUNT 64

#define EVENT_NONE  0
#define EVENT_READ  1
#define EVENT_WRITE 2

#define EVENT_PRINT() 


typedef void (*event_func_t)(int fd,int type,void* args);

extern event_model_t;

typedef struct event_active_s {
    int fd;
    int type;
} event_active_t;

#pragma pack(4)  
typedef struct event_instance_s{
    int            fd;//hash key
    int            type;
    event_func_t   func;
    void *         args;
    UT_hash_handle hh;
}event_instance_t;

typedef struct event_control_s {
    pthread_mutex_t mutex;
    int run;
    int nevents;
    int mem_alloc_count;
    
    event_model_t*    event_model;
    event_active_t*   event_active;
    event_instance_t* event_hash;
    event_instance_t* event_unused;
    event_instance_t* mem_alloc_array;
}event_control_t;
#pragma pack(pop) 


static void init_event_instance_list(event_instance_t* head,int num);
static event_instance_t* alloc_event_instance(event_control_t* eventctr);
static void free_event_instance(event_control_t* eventctr,event_instance_t * eit);

static int add_event_instance(event_control_t* eventctr, int evfd, int evtype,event_func_t *evfunc, void *evargs);
static void del_event_instance(event_control_t* eventctr, int key);
static event_instance_t* find_event_instance(event_control_t* eventctr, int key);

static int event_resize(event_control_t* eventctr, int resize);


extern event_control_t* event_control_create(int size=EVENT_INIT_COUNT);
extern void event_control_destory(event_control_t* eventctr);
extern void event_stop(event_control_t* eventctr);
extern int event_add(event_control_t* eventctr, int fd, int type,event_func_t *func, void *args);
extern void event_mod(event_control_t* eventctr, int fd, int type);
extern int event_ioop(event_control_t* eventctr);


#endif

