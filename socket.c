/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */


#include <netdb.h>

#include "socket.h"


int new_socket(struct addrinfo *ai)
{
    int sfd;
    if ((sfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1) {
        perror("socket");
        return -1;
    }
    return sfd;
}

void close_socket(int sfd)
{
    close(sfd);
}

int set_nonblock_socket(int sfd)
{
    int flags;
    if ((flags = fcntl(sfd, F_GETFL, 0)) == -1)
    {
        perror("fcntl:F_GETFL");
        return -1;
    }

    if(fcntl(sfd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl:F_SETFL");
        return -1;
    }
    return 0;
}


void  set_socket_opts(int sfd)
{
    int ret;
    int flags = 1;
    struct linger ling = {0, 0};
    ret = setsockopt(sfd, IPPROTO_IPV6, IPV6_V6ONLY, (char *) &flags, sizeof(flags));
    if (ret != 0) {
        perror("setsockopt");
    }
    ret = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
    if (ret != 0){
        perror("setsockopt");
    }
    ret = setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags));
    if (ret != 0){
        perror("setsockopt");
    }
    ret = setsockopt(sfd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));
    if (ret != 0){
        perror("setsockopt");
    }
    ret = setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));
    if (ret != 0){
        perror("setsockopt");
    }
    return;
}


int accept_new_socket(int sfd, struct sockaddr *saddr, socklen_t len)
{
    int fd;
    while(1) {
        fd = accept(sfd,saddr,len);
        if (fd == -1) {
            //Interrupted system call
            if (errno == EINTR)
                continue;
            else {
                perror("accpet");
                return -1;
            }
        }
        break;
    }
    return fd;
}


int bind_socket(int sfd, struct addrinfo * saddr)
{
    if (bind(sfd, saddr->ai_addr, saddr->ai_addrlen) == -1) 
    {
        perror("bind");
        return -1;
    }
    
    return 0;
}

int listen_socket(int sfd, int backlog)
{
    if (listen(sfd, backlog) == -1) {
        perror("listen");
        return -1;
    }
    return 0;
}

int send_sock(int fd, char *buf, int size)
{
    return write(fd, buf, size);
}

int recv_sock(int fd, char *buf, int size)
{ 
    return read(fd, buf, size);
}





