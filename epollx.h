/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */
#ifndef __EPOLL_H__
#define __EPOLL_H__

#include <sys/epoll.h>
#include "event.h"

typedef struct event_model_s{
    int epoll_fd;
    struct epoll_event *events;
}event_model_t;


extern int create_event_model(event_control_t* eventctr);
extern void destory_event_model(event_control_t* eventctr);

extern int register_event(event_control_t* eventctr, int fd, int type);
extern int modify_event(event_control_t* eventctr, int fd, int type);
extern int expand_event(event_control_t* eventctr, int resize);

extern int event_model_loop(event_control_t* eventctr, struct timeval *tvp = NULL);

#endif 

