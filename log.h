/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */

#ifndef __LOG_H__
#define __LOG_H__

#include "thread.h"

#define INFO       0
#define DEBUG      1
#define WARNING    2
#define ERROR      3

#define LOG_FILE_NAME_LENGTH 32
#define LOG_FILE_CLOSE_SIZE  10485760 //10M
#define LOG_FILE_RW_BUFFER_SIZE 1024

typedef struct log_s{
       FILE* logfd;
       int   loglevel;
       long  logsize;
       char* logbuf;
       char  logfname[LOG_FILE_NAME_LENGTH];
}log_t;


extern log_t* create_logger(char* log_base_name, int loglevel);
extern void destory_logger(log_t * plgt);


#define logprint(level, fmt, ...) \
    do { \
        thread_t * ptd = (thread_t *)pthread_getspecific(thread_key);\
        assert(ptd);   \
        if(ptd->logt->loglevel > level){ \
             va_list ap;\
             time_t nowtime = time(NULL);\
             tm *now = localtime(&nowtime);\
             va_start(ap, fmt);\
             int res1 = snprintf(ptd->logt->logbuf, LOG_FILE_RW_BUFFER_SIZE ,"%s:%d:%04d-%02d-%02d %02d:%02d:%02d: %s", __FILE__ ,__LINE__ ,\
             now->tm_year+1900, now->tm_mon+1, now->tm_mday,\   
             now->tm_hour, now->tm_min, now->tm_sec);\
             res2 = vsnprintf(ptd->logt->logbuf + res1,LOG_FILE_RW_BUFFER_SIZE - res1,fmt,ap);\
             fwrite(ptd->logt->logfd, ptd->logt->logbuf);\
             fwrite("\n");\
             va_end(ap);\
        }\
    } while (0);\




#endif



