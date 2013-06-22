#include "common.h"

ssize_t writen(int fd, const void * vptr, size_t n)
{
    size_t    nleft;
    ssize_t    nwritten;
    const char *    ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR) {
                nwritten = 0;
            } else {
                return -1;
            }
        }

        nleft -= nwritten;
        ptr += nwritten;
    }

    return n;
}

ssize_t readline(int fd, void * vptr, size_t maxlen)
{
    ssize_t    n, rc;
    char    c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
again:
        if ((rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n') {
                break;
            }
        } else if (rc == 0) {    /* EOF */
            *ptr = 0;
            return (n - 1);
        } else {
            if (errno == EINTR) {
                goto again;
            }
            return (-1);
        }
    }

    *ptr = 0;
    return (n);
}

int Socket(int domain, int type, int protocol)
{
    int sockfd;
    if ((sockfd = socket(domain, type, protocol)) < 0) {
        fprintf(stderr, "socket error\n");
        exit(1);
    }

    return sockfd;
}

int Accept(int sockfd, struct sockaddr * addr, socklen_t * addrlen)
{
    int ret;
    if ((ret = accept(sockfd, addr, addrlen)) < 0) {
        fprintf(stderr, "accept error\n");
        exit(1);
    }

    return ret;
}

int Inet_pton(int af, const char *src, void *dst)
{
    int ret;
    if ((ret = inet_pton(af, src, dst)) < 0) {
        fprintf(stderr, "inet_pton error for %s\n", src);
        exit(1);
    }

    return ret;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int ret;
    if ((ret = connect(sockfd, addr, addrlen)) < 0) {
        fprintf(stderr, "connect error\n");
        exit(1);
    }

    return ret;
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int ret;
    if ((ret = bind(sockfd, addr, addrlen)) < 0) {
        fprintf(stderr, "bind error\n");
        exit(1);
    }

    return ret;
}

int Listen(int sockfd, int backlog)
{
    int ret;
    if ((ret = listen(sockfd, backlog)) < 0) {
        fprintf(stderr, "listen error\n");
        exit(1);
    }

    return ret;
}

int Close(int fd)
{
    int ret;
    if ((ret = close(fd)) < 0) {
        fprintf(stderr, "close error\n");
        exit(1);
    }

    return ret;
}

sig_t Signal(int signum, sig_t handler)
{
    struct sigaction act, oact;

    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(signum, &act, &oact) < 0) {
        return (SIG_ERR);
    }

    return (oact.sa_handler);
}
