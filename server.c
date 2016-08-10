/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */


#include "server.h"


context_t g_context;

void init_context()
{
    get_system_info(&g_context.env);
    g_context.config.daemon_flag = false;
    g_context.config.ipaddr = NULL;
    g_context.config.port = 9728;
    g_context.config.log_base_name = "NetReactor";
    g_context.config.nthreads = g_context.env.cpu_core_num;
    g_context.config.verbose_flag = false;
}


void destory_context()
{





}

void unlimit_core_file()
{
    struct rlimit rlim;
    struct rlimit rlim_new;
    if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
        rlim_new.rlim_cur = rlim_new.rlim_max = RLIM_INFINITY;
        if (setrlimit(RLIMIT_CORE, &rlim_new)!= 0) {
            rlim_new.rlim_cur = rlim_new.rlim_max = rlim.rlim_max;
            setrlimit(RLIMIT_CORE, &rlim_new);
        }
    }
    if ((getrlimit(RLIMIT_CORE, &rlim) != 0) || rlim.rlim_cur == 0) {
        exit(EXIT_FAILURE);
    }
}

void daemonize()
{
    int  fd;
    
    switch (fork()) 
    {
        case -1:
            return -1;
        case 0:
            break;
        default:
            exit(EXIT_SUCCESS);
    }

    if (setsid() == -1) 
    {
        return -1;
    }

    if(chdir("/") != 0) 
    {
        return (-1);
    }

    umask(0);

    fd = open("/dev/null", O_RDWR);
    if (fd == -1) 
    {
        return -1;
    }

    if (dup2(fd, STDIN_FILENO) == -1){
        return -1;
    }

    if (dup2(fd, STDOUT_FILENO) == -1){
        return -1;
    }

    if(dup2(fd, STDERR_FILENO) < 0){
        return (-1);
    }

    if (fd > STDERR_FILENO) {
        if (close(fd) == -1) {
            return -1;
        }
    }

    return 0;
}



void sub_reactor_work_thread_init(context_t* pctx, int nthreads)
{
    int i,ret;
    pctx->threads = xcalloc(nthreads, sizeof(thread_t));
    if (pctx->threads != NULL) {
        perror("Can't allocate thread descriptors");
        exit(EXIT_FAILURE);
    }
    
    for (i = 0; i < nthreads; i++) {
        int fds[2];
        if (pipe(fds) == -1) {
            perror("Can't create notify pipe");
            exit(EXIT_FAILURE);
        }

        pctx->threads[i]->notify_recv_fd = fds[0];
        pctx->threads[i]->notify_send_fd = fds[1];
        pctx->threads[i]->conn.conn_active = NULL;
        pctx->threads[i]->conn.conn_unused = NULL;
        pctx->threads[i]->conn.conn_queue = NULL;
        pctx->threads[i]->conn_num = 0;
        
        pctx->threads[i]->logt = NULL;

        pthread_mutex_init(&(pctx->threads[i]->conn.conn_queue_lock),NULL);

        if(add_notify_event(pctx->threads[i]) == -1){
            perror("Can't add event");
            exit(EXIT_FAILURE);
        }
    }

    ret = pthread_key_create(&thread_key, NULL);     
    if(ret != 0)  
    {  
        fprintf(stderr, "pthread key create fail\n");  
        exit(EXIT_FAILURE);
    }  
  
    int cpunum = pctx->env.cpu_core_num;
    
    for (i = 0; i < nthreads; i++) {
        ret = create_thread(pctx->threads + i, sub_reactor_work_thread, pctx->threads + i);
        if (ret == -1)
        {
            perror("Can't create sub reactor thread");
            exit(EXIT_FAILURE);
        }
        ret = bind_cpu_core(i % cpunum,pctx->threads + i)
        if (ret == -1)
        {
            perror("Can't bind cpu core");
            exit(EXIT_FAILURE);
        }
    }
    wait_for_thread_startup(nthreads);
    return;

}

void sub_reactor_work_thread(void* args)
{
    int ret;
    thread_t * ptd = (thread_t *)args;
    assert(ptd != NULL);
    
    ptd->logfd = create_logger(g_context.config.log_base_name,g_context.config.loglevel);
    assert(ptd->logfd);
    
    if((ret = pthread_setspecific(thread_key,ptd)) != 0)
    {
        perror("pthread_setspecific");
        exit(EXIT_FAILURE);
    }
    thread_startup_over();
    
    sub_reactor_io_loop(ptd);
    return;
}


void sub_reactor_io_loop(thread_t* ptd)
{
    assert(ptd);

    if(event_ioop(ptd->eventctr) == 0){
        fprintf(stderr, "sub_reactor_work_thread: exit event loop normally\n");
    }else{
        fprintf(stderr, "sub_reactor_work_thread: exit event loop exceptionally\n");
    }
    return;
}


void main_reactor_io_loop(context_t * ctx)
{
    assert(ctx);

    if(event_ioop(ctx->event_main) == 0){
        fprintf(stderr, "main_reactor_io_loop: exit event loop normally\n");
    }else{
        fprintf(stderr, "main_reactor_io_loop: exit event loop exceptionally\n");
    }
    
    return;
}


int main(int argc, char * argv[])
{
    init_context();
    parse_args(argc,argv,cfg);
    unlimit_core_file();
    
    daemonize();
    sub_reactor_work_thread_init();
    
    main_reactor_io_loop();

    destory_context();
    return 0;

}







