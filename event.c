/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */

#include <assert.h>

#include "event.h"

//由于uthash结构是内嵌式的，所以该结构也能形成一个单向列表
//借用uthash结构中的UT_hash_handle的next指针，形成一个单向列表
INLINE void init_event_instance_list(event_instance_t* head,int num)
{
    assert(head);
    assert(num > 0);
    for(int i = 0;i < num; ++i)
    {
        if(i == num - 1)
        {
            head[i].hh.next = NULL;
            return;
        }
        head[i].hh.next = head + i + 1;
    }
    return;
}
INLINE event_instance_t* alloc_event_instance(event_control_t* eventctr)
{
    assert(eventctr);
    if(eventctr->event_unused == NULL)
    {
        eventctr->event_unused = xcalloc(EVENT_DELTA_COUNT,sizeof(event_instance_t));
        if(eventctr->event_unused == NULL)
        {
            return NULL;
        }
        eventctr->mem_alloc_array[eventctr->mem_alloc_count] =  eventctr->event_unused;
        eventctr->mem_alloc_count++;
        
        init_event_instance_list(eventctr->event_unused,EVENT_DELTA_COUNT);
    }
    event_instance_t tmp;
    tmp = eventctr->event_unused;
    eventctr->event_unused = eventctr->event_unused->hh.next;
    return tmp;
}

INLINE void free_event_instance(event_control_t* eventctr,event_instance_t * einst)
{
    assert(eventctr);
    assert(einst);
    einst->hh.next = eventctr->event_unused;
    eventctr->event_unused = einst;
    return;
}
INLINE int add_event_instance(event_control_t* eventctr, int evfd, int evtype,event_func_t *evfunc, void *evargs)
{
    assert(eventctr);
    event_instance_t* newevent = alloc_event_instance(eventctr);
    if(newevent == NULL) return -1;
    newevent->fd = evfd;
    newevent->type |= evtype;
    newevent->func = evfunc;
    newevent->args = evargs;
    HASH_ADD_INT(eventctr->event_hash, fd, newevent);
    return 0;
}
INLINE event_instance_t* find_event_instance(event_control_t* eventctr, int key)
{
    assert(eventctr);
    event_instance_t *eit;
    HASH_FIND_INT(eventctr->event_hash, &key, eit);
    return eit;
}

INLINE void del_event_instance(event_control_t* eventctr, int key)
{
    assert(eventctr);
    event_instance_t *tmp;
    tmp = find_event_instance(eventctr, key);
    if(tmp != NULL) HASH_DEL(eventctr->event_hash ,tmp);
    free_event_instance(tmp);
    return;
}

event_control_t* event_control_create(int num)
{
    int ret;
    event_control_t * eventctr;
    eventctr = xmalloc(sizeof(*eventctr));
    if(eventctr == NULL)
    {   
        return NULL;
    }
    
    pthread_mutex_init(&eventctr->mutex,NULL);

    eventctr->mem_alloc_array = xcalloc(EVENT_DELTA_MEM_ALLOC_MAX_COUNT, sizeof(event_instance_t*));
    if(eventctr->mem_alloc_array == NULL){
        return NULL;
    }
    
    eventctr->event_hash = NULL;
    eventctr->event_unused = xcalloc(num, sizeof(event_instance_t));
    eventctr->event_active = xcalloc(num, sizeof(event_active_t));
    if(eventctr->event_unused == NULL || eventctr->event_active == NULL)
    {   
        return NULL;
    }
    eventctr->mem_alloc_count = 0;
    eventctr->mem_alloc_array[eventctr->mem_alloc_count] =  eventctr->event_unused;
    eventctr->mem_alloc_count++;
    
    eventctr->nevents = num;
    eventctr->run = 1;

    ret = create_event_model(eventctr)
    if(ret == -1)
    {
        return NULL;
    }
    return eventctr;

}

void event_control_destory(event_control_t* eventctr)
{
    int i;
    destory_event_model(eventctr);
    for(i = 0; i < eventctr->mem_alloc_count; ++i){
        xfree(eventctr->mem_alloc_array[i]);
    }
    xfree(eventctr->event_active);
    xfree(eventctr);
}

void event_stop(event_control_t* eventctr)
{
    eventctr->run = 0;
}

int event_add(event_control_t* eventctr, int fd, int type,event_func_t *func, void *args)
{
    pthread_mutex_lock(&eventctr->mutex);  
    if (register_event(eventctr,fd,type) == -1)
    {
        pthread_mutex_unlock(&eventctr->mutex); 
        return -1;
    }
    if(add_event_instance(eventctr,fd,type,func,args) == -1)
    {
        pthread_mutex_unlock(&eventctr->mutex); 
        return -1;
    }
    pthread_mutex_unlock(&eventctr->mutex); 
    return 0;
}

extern void event_mod(event_control_t* eventctr, int fd, int type)
{
    event_instance_t* tmp;
    pthread_mutex_lock(&eventctr->mutex); 
    tmp = find_event_instance(eventctr,fd);
    if(tmp == NULL)
    {
        pthread_mutex_unlock(&eventctr->mutex);
        return -1;
    }
    tmp->type = type;
    if(modify_event(eventctr,fd,type) == -1)
    {
        pthread_mutex_unlock(&eventctr->mutex);
        return -1;
    }
    if(type == EVENT_NONE)
    {
        del_event_instance(eventctr,tmp);
    }
    pthread_mutex_unlock(&eventctr->mutex);
    return 0;
}

int event_resize(event_control_t* eventctr, int resize)
{
    assert(eventctr);

    if (expand_event(eventctr,resize) == -1) return -1;
    eventctr->event_active = xrealloc(eventctr->event_active,sizeof(event_active_t) * resize);
    if(eventctr->event_active == NULL)
    {   
        return -1;
    }
    eventctr->nevents = resize;
    return 0;
}

int event_ioop(event_control_t* eventctr)
{
    assert(eventctr);
    int ret,i,fd,type,nevents;
    
    eventctr->run = 1;
    
    while(eventctr->run)
    {
        pthread_mutex_lock(&eventctr->mutex);
        nevents = event_model_loop(eventctr,NULL);
        pthread_mutex_unlock(&eventctr->mutex);

        for (i = 0; i < nevents; ++i) 
        {
            fd = eventctr->event_active[i].fd;
            type = eventctr->event_active[i].type;
            
            event_instance_t *e = find_event_instance(eventctr,fd);

            if (e->type & (EVENT_READ | EVENT_WRITE)) 
            {
                e->func(fd,type,e->args);
            }
        }
        
        if (eventctr->nevents - nevents <= EVENT_THRESHOLD_COUNT)
        {
            ret = event_resize(eventctr, eventctr->nevents + EVENT_DELTA_COUNT);
            if(ret == -1)
            {
                fprintf(stderr, "event_resize:cannot alloc memroy,exit event loop\n");
                return -1;
            }
        }
    }
    return 0;
}

 

