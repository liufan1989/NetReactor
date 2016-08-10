/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */

#ifndef __NET_H__
#define __NET_H__


#pragma pack(4)  
typedef struct conn_node_s{
    int    sfd;
    int    etype;
    int    port;
    char  *ipaddr;

    int    rbufsize;   
    int    readpos;
    int    wbufsize;   
    int    writepos;

    char  *readbuf;   
    char  *writebuf;   

    UT_hash_handle hh;
}conn_node_t;
#pragma pack(pop) 


typedef struct conn_queue_s{
    conn_node_t* connode;
    struct conn_queue_s* next;
}conn_queue_t;


typedef struct conn_s{
    int conn_num;                    //total connect node number
    conn_node_t* conn_active;        //current connect number
    conn_node_t* conn_unused;        // free connect number
    conn_queue_t* conn_queue;        // connect queue
    pthread_mutex_t conn_queue_lock; // queue lock
}conn_t;

static void queue_push(conn_t *conn,conn_node_t* connode);
static conn_node_t* queue_pop(conn_t *conn);

static conn_node_t* alloc_connection(context_t* ctx, int cfd, int type, char* ipaddr, int port);
static void free_connection(context_t ctx,conn_node_t* cnode);


static int add_notify_event(thread_t* pthd);
static thread_t* dispatch_thread(context_t* ctx);
static void dispatch_new_connect(context_t* ctx,conn_t* con);
static void handle_thread_connect(int fd, int type, void * args);
static  void handle_read_write(int fd,int type,void* args);
static  void handle_accept_connect(int fd,int type,void* args);


extern int init_server(context_t context, char* bind_addr=NULL, int port=9728, int backlog=1024);
#endif


 

