/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */

#ifndef __ENV_H__
#define __ENV_H__

#include <stdio.h>
#include <unistd.h>
#include "env.h"


void get_system_info(env_t * env)
{
    env->cpu_core_num = sysconf(_SC_NPROCESSORS_CONF);
    env->mem_page_size = sysconf(_SC_PAGESIZE);
    return;
}



#endif


