#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define LISTENQ 20
#define MAXLINE 40
#define BUFFERSIZE 4096
#define PORT 50000

ssize_t writen(int fd, const void * vptr, size_t n);
ssize_t readline(int fd, void * vptr, size_t maxlen);

//wrappers

int Socket(int domain, int type, int protocol);
int Accept(int sockfd, struct sockaddr * addr, socklen_t * addrlen);
int Inet_pton(int af, const char *src, void *dst);
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Listen(int sockfd, int backlog);
int Close(int fd);
sig_t Signal(int signum, sig_t handler);

#endif
