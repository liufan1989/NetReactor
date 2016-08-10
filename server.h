/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */

#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdlib.h>

#include "env.h"
#include "thread.h"
#include "config.h"
#include "event.h"
#include "log.h"

typedef struct context_s{
    int listen_fd;
    env_t env;
    config_t  config;
    thread_t* threads;
    event_control_t* event_main;
    stats_t* stats;
    log_t* log;
}context_t;


#endif 



