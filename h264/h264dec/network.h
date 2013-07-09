/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <stdint.h>

#ifndef NETWORK_H
#define NETWORK_H

int net_tcp_connect(char *host, unsigned long port);
int net_udp_connect(char *host, unsigned long port);
int net_unix_sock(const char *path);
int net_sock_nonblock(int sockfd);
int net_sock_cork(int fd, int state);
int net_send16(int fd, uint16_t n);
int net_send32(int fd, uint32_t n);

#endif
