/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */
#include <assert.h>
#include "epollx.h" //区分和系统文件混淆

int create_event_model(event_control_t* eventctr)
{
    assert(eventctr != NULL);
    event_model_t *epollmodel = xcalloc(1,sizeof(event_model_t));
    if(epollmodel == NULL)
    {
        return -1;
    }
    epollmodel->events = xmalloc(sizeof(struct epoll_event) * eventctr->nevents);
    if (epollmodel->events == NULL) 
    {
        xfree(epollmodel);
        return -1;
    }
    epollmodel->epoll_fd = epoll_create(1024);
    if (epollmodel->epoll_fd == -1) 
    {
        xfree(epollmodel->events);
        xfree(epollmodel);
        return -1;
    }
    eventctr->event_model = epollmodel;
    return 0;
}

void destory_event_model(event_control_t* eventctr)
{
    assert(eventctr != NULL);
    event_model_t * epollmodel = eventctr->event_model;
    close(epollmodel->epoll_fd);
    xfree(epollmodel->events);
    xfree(epollmodel);
    return;
}

int register_event(event_control_t* eventctr, int fd, int type)
{
    assert(eventctr != NULL);
    event_model_t * epollmodel = eventctr->event_model;    
    struct epoll_event new_event = {0};
    new_event.data.fd = fd;
    
    if (type & EVENT_READ) new_event.events |= EPOLLIN;
    if (type & EVENT_WRITE) new_event.events |= EPOLLOUT;

    if (epoll_ctl(epollmodel->epoll_fd,EPOLL_CTL_ADD,fd,&new_event) == -1){
        perror("epoll_ctl:EPOLL_CTL_ADD");
        return -1;
    }
    return 0;
}

int modify_event(event_control_t* eventctr, int fd, int type)
{
    assert(eventctr != NULL);
    event_model_t * epollmodel = eventctr->event_model;
    if(type == EVENT_NONE)
    {
        if(epoll_ctl(epollmodel->epoll_fd,EPOLL_CTL_DEL,fd,NULL) == -1){
            perror("epoll_ctl:EPOLL_CTL_DEL");
            return -1;
        }
        return 0;  
    }
    else
    {
        struct epoll_event new_event = {0};
        new_event.data.fd = fd;
        if (type & EVENT_READ) new_event.events |= EPOLLIN;
        if (type & EVENT_WRITE) new_event.events |= EPOLLOUT;
        if (epoll_ctl(epollmodel->epoll_fd,EPOLL_CTL_MOD,fd,&new_event) == -1){
            perror("epoll_ctl:EPOLL_CTL_MOD");
            return -1;
        }
        return 0;
    }

}

int expand_event(event_control_t* eventctr, int resize)
{
    assert(eventctr != NULL);
    event_model_t * epollmodel = eventctr->event_model;
    epollmodel->events = xrealloc(epollmodel->events, sizeof(struct epoll_event) * resize);
    if(epollmodel->events == NULL) return -1;
    return 0;
}

int event_model_loop(event_control_t* eventctr, struct timeval *tvp)
{
    assert(eventctr != NULL);
    event_model_t * epollmodel = eventctr->event_model;
    assert(epollmodel != NULL);
    
    int i,ret,nevents,mask;
    struct epoll_event *e;

    ret = epoll_wait(epollmodel->epoll_fd,epollmodel->events,eventctr->nevents,
            tvp != NULL ? (tvp->tv_sec * 1000 + tvp->tv_usec / 1000) : -1);
            
    if (ret > 0) 
    {
        nevents = ret;
        for (i = 0; i < nevents; ++i) 
        {
            mask = 0;
            e = epollmodel->events + i;
            
            if (e->events & EPOLLIN ) mask |= EVENT_READ;
            if (e->events & EPOLLPRI) mask |= EVENT_READ;
            if (e->events & EPOLLOUT) mask |= EVENT_WRITE;
            if (e->events & EPOLLERR) mask |= EVENT_WRITE;
            if (e->events & EPOLLHUP) mask |= EVENT_WRITE;
            
            eventctr->event_active[i].fd = e->data.fd;
            eventctr->event_active[i].type = mask;
        }
        return nevents;
    }
    
    if (ret == 0) return 0;
    if (ret < 0) return -1;

}



 

