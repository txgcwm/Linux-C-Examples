/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <syscall.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "network.h"
#include "streamer.h"
#include "utils.h"

/*
 * Register stream data into local file system, create the unix
 * socket and return the file descriptor.
 */
int streamer_prepare(const char *name,
                     unsigned char *sps, int sps_len,
                     unsigned char *pps, int pps_len)
{
    int fd;
    int size = 128;
    char path_sock[size];
    char path_meta[size];
    uint8_t nal_header[4] = {0x00, 0x00, 0x00, 0x01};

    snprintf(path_sock, size, "/tmp/%s.h264s.sock", name);
    snprintf(path_meta, size, "/tmp/%s.h264s.meta", name);

    /* write metadata file */
    fd = open(path_meta, O_CREAT|O_WRONLY|O_TRUNC, 0666);
    if (fd <= 0) {
        ERR();
        exit(EXIT_FAILURE);
    }

    write(fd, &nal_header, sizeof(nal_header));
    write(fd, sps, sps_len);
    write(fd, &nal_header, sizeof(nal_header));
    write(fd, pps, pps_len);
    close(fd);

    /* create unix sock */
    fd = net_unix_sock(path_sock);

    return fd;
}

/*
 * Create the local Pipe, this pipe is used to transfer the
 * extracted H264 data to the unix socket. It also takes care
 * to increase the pipe buffer size.
 */
int streamer_pipe_init(int pipefd[2])
{
    int fd;
    int ret;
    int size = 64;
    long pipe_max;
    char buf[size];

    /* create pipe */
    ret = pipe(pipefd);
    if (ret != 0) {
        printf("Error: could not create streamer pipe\n");
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    /* Set non blocking mode */
    net_sock_nonblock(pipefd[0]);
    net_sock_nonblock(pipefd[1]);

    /* Get maximum pipe buffer size allowed by the kernel */
    fd = open("/proc/sys/fs/pipe-max-size", O_RDONLY);
    if (fd <= 0) {
        printf("Warning: could not open pipe-max-size");
    }
    else {
        ret = read(fd, buf, size);
        if (ret <= 0) {
            printf("Warning: could not read pipe-max-size value");
            perror("read");
            exit(EXIT_FAILURE);
        }

        close(fd);

        pipe_max = atol(buf);
        ret = fcntl(pipefd[1], F_SETPIPE_SZ, pipe_max);

        if (ret == -1) {
            printf("Warning: could not increase pipe limit to %lu\n", pipe_max);
            exit(EXIT_FAILURE);
        }

        return 0;
    }

    return -1;
}

void *streamer_worker(void *arg)
{
    int bytes;
    int remote_fd;
    int server_fd;
    int size = 650000;
    char buf[size];
    struct stream_w *sw;
    struct sockaddr_un address;
    socklen_t socket_size = sizeof(struct sockaddr_un);


    sw = arg;
    sw->task_id = syscall(__NR_gettid);
    server_fd = sw->server_fd;

    while (1) {
        remote_fd = accept(server_fd, (struct sockaddr *) &address, &socket_size);
        if (remote_fd <= 0) {
            ERR();
            printf(">> Streamer: closing worker\n");
            pthread_exit(0);

        }
        net_sock_nonblock(remote_fd);

        printf("new connection: %i\n", remote_fd);

        while (1) {
            memset(buf, '\0', sizeof(buf));
            bytes = read(stream_pipe[0], buf, sizeof(buf));
            if (bytes > 0) {
                send(remote_fd, buf, bytes, 0);
            }
            else if (bytes == -1) {
                sleep(0.5);
                continue;
            }
            send(remote_fd, buf, bytes, 0);
        }

        close(remote_fd);
    }
}

pid_t streamer_loop(int server_fd)
{
    pthread_t tid;
    pthread_attr_t thread_attr;
    struct stream_w *sw;

    printf("1 server_fd = %i\n", server_fd);
    sw = malloc(sizeof(struct stream_w));
    sw->server_fd = server_fd;
    sw->task_id   = 0;

    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&tid, &thread_attr,
                       streamer_worker, (void *) sw) < 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    return sw->task_id;
}

int streamer_write_h264_header(unsigned char *sps_dec, size_t sps_len,
                               unsigned char *pps_dec, size_t pps_len)
{
     uint8_t nal_header[4] = {0x00, 0x00, 0x00, 0x01};

     /* [00 00 00 01] [SPS] */
     write(stream_fs_fd, &nal_header, sizeof(nal_header));
     write(stream_fs_fd, sps_dec, sps_len);

     /* [00 00 00 01] [PPS] */
     write(stream_fs_fd, &nal_header, sizeof(nal_header));
     write(stream_fs_fd, pps_dec, pps_len);

     return 0;
}

/* write data to unix socket */
int streamer_write(const void *buf, size_t count)
{
    /* write to file system debug file */
    if (stream_dump) {
        write(stream_fs_fd, buf, count);
    }

    /* write to pipe */
    return write(stream_pipe[1], buf, count);
}

int streamer_write_nal()
{
    uint8_t nal_header[4] = {0x00, 0x00, 0x00, 0x01};

    /* write header to file system debug file */
    if (stream_dump) {
        write(stream_fs_fd, &nal_header, sizeof(nal_header));
    }

    return write(stream_pipe[1], &nal_header, sizeof(nal_header));
}
