/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */

#ifndef __ENV_H__
#define __ENV_H__


typedef struct env_s{
    long  cpu_core_num;
    long  mem_page_size;
}env_t;


extern void get_system_info(env_t* env);

#endif


