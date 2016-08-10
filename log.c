/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */

#include "log.h"
#include "thread.h"



log_t* create_logger(char* log_base_name, int loglevel)
{
    pthread_t tid = pthread_self();
    thread_t * pthd = (thread_t *)pthread_getspecific(thread_key);
    log_t* plgt = xmalloc(sizeof(log_t));
    if(plgt == NULL)
    {
        return NULL;
    }
    plgt->logbuf = xmalloc(LOG_FILE_RW_BUFFER_SIZE);
    if(plgt->logbuf == NULL)
    {
        return NULL;
    }
    plgt->loglevel = loglevel;
    plgt->logsize = 0;
    snprintf(plgt->logfname, LOG_FILE_NAME_LENGTH, "%s_%ld.log", log_base_name, tid);
    plgt->logfd = fopen(plgt->logfname, "a+");
    if(plgt->logfd == NULL){
        return NULL;
    }
    return plgt;
}


void destory_logger(log_t * plgt)
{
    assert(plgt);
    xfree(plgt->logbuf);
    fclose(plgt->logfd);
    xfree(plgt);
    return;
}




