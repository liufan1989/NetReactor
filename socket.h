/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */



#ifndef __SOCKET_H__
#define __SOCKET_H__


#include <sys/socket.h>


extern int bind_socket(int sfd, struct addrinfo *saddr);

extern int listen_socket(int sfd, int backlog);

extern int accept_new_socket(int sfd, struct sockaddr *saddr, socklen_t len);

extern int set_nonblock_socket(int sfd);

extern int new_socket(struct addrinfo *ai);

extern void close_socket(int sfd);

extern int set_socket_opts(int sfd);

extern int send_sock(int fd, char *buf, int size);

extern int recv_sock(int fd, char *buf, int size);


#endif







