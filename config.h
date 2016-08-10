/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */
#ifndef __CONFIG_H__
#define __CONFIG_H__


extern stats_t g_stats;

typedef struct config_s{
    char * ipaddr;
    char * log_base_name;
    char * delimiter;
    char * username;
    
    int port;
    int nthreads;
    int backlog;
    int loglevel;

    bool daemon_flag;
    bool core_flag;
    bool verbose_flag;

    char* version;
    
}config_t;


typedef struct stats_s{
    long close_count;
    long accept_count;
    long connect_count;
   // long mem_alloc_count;
   // long mem_free_count;
   // long mem_malloc_size;
   // long mem_calloc_size;
   // long mem_realloc_size;
}stats_t;




