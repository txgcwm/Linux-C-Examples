/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  RTSP Client
 *  -----------
 *  Written by Eduardo Silva P. <edsiper@gmail.com>
 */

/* generic */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

/* networking */
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

/* Connect to a TCP socket server and returns the file descriptor */
int net_tcp_connect(char *host, unsigned long port)
{
    int res;
    int sock_fd;
    struct sockaddr_in *remote;

    sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_fd <= 0) {
	    printf("Error: could not create socket\n");
	    return -1;
    }

    remote = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    remote->sin_family = AF_INET;
    res = inet_pton(AF_INET, host, (void *) (&(remote->sin_addr.s_addr)));

    if (res < 0) {
	    printf("Error: Can't set remote->sin_addr.s_addr\n");
	    free(remote);
	    return -1;
    }
    else if (res == 0) {
	    printf("Error: Invalid address '%s'\n", host);
	    free(remote);
	    return -1;
    }

    remote->sin_port = htons(port);
    if (connect(sock_fd,
                (struct sockaddr *) remote, sizeof(struct sockaddr)) == -1) {
        close(sock_fd);
        printf("Error connecting to %s:%lu\n", host, port);
        free(remote);
        return -1;
    }

    free(remote);
    return sock_fd;
}


/* Connect to a UDP socket server and returns the file descriptor */
int net_udp_connect(char *host, unsigned long port)
{
    int res;
    int sock_fd;
    struct sockaddr_in *remote;

    sock_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd <= 0) {
	    printf("Error: could not create socket\n");
	    return -1;
    }

    remote = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    remote->sin_family = AF_INET;
    res = inet_pton(AF_INET, host, (void *) (&(remote->sin_addr.s_addr)));

    if (res < 0) {
	    printf("Error: Can't set remote->sin_addr.s_addr\n");
	    free(remote);
	    return -1;
    }
    else if (res == 0) {
	    printf("Error: Invalid address '%s'\n", host);
	    free(remote);
	    return -1;
    }

    remote->sin_port = htons(port);
    if (connect(sock_fd,
                (struct sockaddr *) remote, sizeof(struct sockaddr)) == -1) {
        close(sock_fd);
        printf("Error connecting to %s:%lu\n", host, port);
        free(remote);
        return -1;
    }

    free(remote);
    return sock_fd;
}

int net_unix_sock(const char *path)
{
    int server_fd;
    size_t address_length;
    struct sockaddr_un address;

    /* Create listening socket */
    server_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    /* just in case, remove previous sock */
    unlink(path);

    address.sun_family = AF_UNIX;
    snprintf(address.sun_path, sizeof(address.sun_path), path);
    address_length = sizeof(address.sun_family) + strlen(path);

    if (bind(server_fd, (struct sockaddr *) &address, address_length) != 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) != 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

int net_sock_nonblock(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1) {
        perror("fcntl");
        return -1;
    }

   return 0;
}


int net_sock_cork(int fd, int state)
{
    return setsockopt(fd, IPPROTO_TCP, TCP_CORK, &state, sizeof(state));
}

int net_send16(int fd, uint16_t n)
{
    uint16_t nbo = htons(n);
    return send(fd, &nbo, sizeof(nbo), 0);
}

int net_send32(int fd, uint32_t n)
{
    uint32_t nbo = htonl(n);
    return send(fd, &nbo, sizeof(nbo), 0);
}
