/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */

#include <assert.h>

#include "net.h"
#include "event.h"

void  queue_push(conn_t *connt, conn_node_t* connode)
{
    conn_queue_t* connq = xmalloc(sizeof(conn_queue_t));
    assert(connq);
    pthread_mutex_lock(&connt->conn_queue_lock); 
    connq->connode = connode;
    connq->next = connt->conn_queue;
    connt->conn_queue = connq;
    pthread_mutex_unlock(&connt->conn_queue_lock);

}

conn_node_t* queue_pop(conn_t *connt)
{
    conn_queue_t* connq;
    conn_node_t* node;
    pthread_mutex_lock(&connt->conn_queue_lock); 
    connq = connt->conn_queue;
    connt->conn_queue = connq->next;
    pthread_mutex_unlock(&connt->conn_queue_lock);
    node = connq->connode;
    xfree(connq);
    return node;
}

conn_node_t* alloc_connection(thread_t *pthd, int cfd, int type, char* ipaddr, int ipaddrlen, int port, int mem_page_size){
    assert(pthd);
    conn_node_t* conn;
    if(pthd->conn.conn_unused == NULL){
        pthd->conn.conn_num++;
        conn = xcalloc(1,sizeof(conn_node_t));
        if(conn == NULL){
            fprintf(stderr,"xcalloc(): cannot malloc new connect\n");
            return NULL;
        }
        conn->rbufsize = conn->wbufsize = mem_page_size * 4;
        conn->readbuf = xmalloc(conn->rbufsize);
        conn->writebuf = xmalloc(conn->wbufsize);
        if(conn->readbuf == NULL || conn->writebuf == NULL)
        {
            fprintf(stderr,"xcalloc(): cannot malloc read buffer and write buffer\n");
            return NULL;
        }    
        conn->ipaddr = xstrdup(ipaddr,ipaddrlen);
    }else{
        conn = pthd->conn.conn_unused;
        pthd->conn.conn_unused = pthd->conn.conn_unused->hh.next;
        strncpy(conn->ipaddr,ipaddr,ipaddrlen);
    }
    conn->sfd = cfd;
    conn->etype = type;
    conn->port = port;
    conn->readpos = 0;
    conn->writepos = 0;
    return conn;
    
}

void free_connection(thread_t *pthd,int sfd){
    assert(pthd);
    conn_node_t* conn;
    HASH_FIND_INT(pthd->conn.conn_active, &sfd, conn);
    if(conn != NULL){
        HASH_DEL(pthd->conn.conn_active ,conn);
    }
    conn->hh.next = pthd->conn.conn_unused;
    pthd->conn.conn_unused = conn;
    return;
}


//不可重入函数
thread_t* dispatch_thread(context_t* ctx){
    static int dispatch_index = -1;
    int tid = (dispatch_index + 1) % ctx->config->nthreads;
    thread_t *thread = ctx->threads + tid;
    dispatch_index = tid;
    return thread;
}

int add_notify_event(thread_t* pthd)
{
    assert(pthd);
    pthd->eventctr = event_control_create(EVENT_INIT_COUNT);
    if(pthd->eventctr == NULL)
    {
        return -1;
    }
    if(event_add(pthd->eventctr,pthd->notify_recv_fd,EVENT_READ,handle_thread_connect,pthd) == -1)
    {
        return -1;
    }
    return 0;
}

void dispatch_new_connect(thread_t *pthd, conn_node_t* connode)
{
    char buf[1] = { 'c' };
    queue_push(pthd->conn,connode);
    if (write(pthd->notify_send_fd, buf, 1) != 1) {
        perror("Writing to thread notify pipe for new connection");
    }
    return;
}

void handle_thread_connect(int fd, int type, void * args)
{
    char buf[1];
    conn_node_t* connode;
    assert(args);
    thread_t * pthd = (thread_t*) args;
    if (read(fd, buf, 1) != 1){
        fprintf(stderr, "handle_thread_connect:Can't read from notify pipe\n");
        return;
    }    
    if(buf[0] == 'c')
    {
        connode = queue_pop(&pthd->conn);
        if(connode == NULL)
        {
            fprintf(stderr, "handle_thread_connect:queue_pop fail\n"); 
            return;
        }
        if(event_add(pthd->eventctr,connode->sfd,EVENT_READ,handle_read_write,connode) == -1){
            fprintf(stderr, "handle_thread_connect:event add fail\n");
            return;
        }
            
    }else{
       fprintf(stderr, "handle_thread_connect buf=%c :Read from notify pipe is error\n",buf[0]);
    }
    return;
}

void handle_close(int sfd){
    thread_t pthd = (thread_t*)pthread_getspecific(thread_key);
    assert(pthd);
    close(sfd);
    free_connection(thread_t * pthd,int sfd);
    return;
}

void handle_read_write(int fd,int type,void* args)
{
    assert(args);
    conn_node_t* connode = (conn_node_t*)args;
    assert(connode->sfd == fd);
    
    if(type & EVENT_READ){
        int res,remain;
        while(true){
            res = recv_sock(fd,connode->readbuf,connode->rbufsize)
            if (res > 0) {
                connode->readpos += res;
                if (res == connode->rbufsize) {
                    connode->rbufsize *= 2;
                    connode->readbuf = xrealloc(connode->readbuf,connode->readbuf);
                    continue;
                } else {
                    break;
                }
            }
            if (res == 0) {
                handle_close(fd);
                return -1;
            }
            if (res == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                return 0;
            }
            

            
        }
        write(fd, connode->writebuf, connode->wbufsize- connode->writepos);
    }else if(type & EVENT_WRITE){
        


    }else{
         fprintf(stderr, "handle_read_write event type is error:fd=%d,type=%d\n",fd,type);
    }
    return;
}


void handle_accept_connect(int fd,int type,void* args)
{
    assert(args != NULL);
    char ipaddr[46]={0};
    int cfd,port;
    context_t* ctx;
    struct sockaddr_storage addr;

    ctx = (context_t*)args;
    cfd = accept_new_socket(fd,struct (struct sockaddr*)&addr,sizeof(addr));
    if(cfd == -1){
         fprintf(stderr,"accept_new_socket(): is error\n");
         return;
    }
    if (addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&addr;
        inet_ntop(AF_INET,(void*)&(s->sin_addr),ipaddr,sizeof(ipaddr));
        port = ntohs(s->sin_port);
    } else {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
        inet_ntop(AF_INET6,(void*)&(s->sin6_addr),ipaddr,sizeof(ipaddr));
        port = ntohs(s->sin6_port);
    }
    thread_t * pthd;
    pthd = dispatch_thread(ctx);
    fprintf(stderr,"Clientfd:%d,IP:%s,Port:%d dispatch to thread[%ld]\n",cfd,ipaddr,port,pthd->thread_id);

    conn_node_t* connode;
    connode = alloc_connection(pthd,cfd,type,ipaddr,46,port,ctx->env.mem_page_size);
    assert(connode);
    
    ctx->stats->accept_count++;

    dispatch_new_connect(pthd,connode);
    return;
    
}


int init_server(context_t context, char* bind_addr=NULL, int port=9728, int backlog=1024)
{

    assert(context != NULL);
    int sfd,ret;
    char port_buf[32] = {0};

    struct addrinfo *ai,*rp;
    struct addrinfo hints;

    memset(&hints,0,sizeof(struct addrinfo));
    hints.ai_flags     = AI_PASSIVE;
    hints.ai_family    = AF_UNSPEC;
    hints.ai_socketype = SOCK_TREAM;

    snprintf(port_buf, sizeof(port_buf), "%d", port);

    //获取网卡地址
    ret = getaddrinfo(bind_addr, port_buf, &hints, &ai);
    if (ret != 0) {
        fprintf(stderr,"getaddrinfo(): %s\n",gai_strerror(s));  
        return -1;
    }

    for(rp = ai; rp != NULL; rp = rp->ai_next)
    {
        if(rp->ai_family == AF_INET){
            fprintf(stderr,"Opening ipv4 listen socket on port %d.", ntohs(((struct sockaddr_in *)rp->ai_addr)->sin_port));
        }else if(rp->ai_family == AF_INET6){
            fprintf(stderr,"Opening ipv6 listen socket on port %d.", ntohs(((struct sockaddr_in6 *)rp->ai_addr)->sin6_port));
        }
        else
        {
            fprintf(stderr,"Opening Socket family type is error");
            continue;
        }
        if ((sfd = new_socket(rp)) == -1) 
        {
            continue;
        }
        if(set_nonblock_socket(sfd) ==  -1 || set_socket_opts(sfd) == -1)
        {
            close_socket(sfd);
            return  -1;
        }
        if(bind_socket(sfd,rp) == -1 || listen_socket(sfd,backlog) == -1){
            close_socket(sfd);
            return  -1;
        }
        
        event_add(context->event_main,sfd,EVENT_READ,handle_accept_connect,context);
        
    }
    freeaddrinfo(ai);
    return 0;
 
}


 
