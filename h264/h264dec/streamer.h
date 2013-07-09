/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef STREAMER_H
#define STREAMER_H

#ifndef F_SETPIPE_SZ
#define F_SETPIPE_SZ	1031
#endif

int stream_sock;
int stream_pipe[2];
int stream_fs_fd;
char *stream_dump;

struct stream_w {
    int   server_fd;
    pid_t task_id;
};

pid_t streamer_loop(int server_fd);
int streamer_prepare(const char *name,
                     unsigned char *sps, int sps_len,
                     unsigned char *pps, int pps_len);
int streamer_pipe_init(int pipefd[2]);
int streamer_write(const void *buf, size_t count);
int streamer_write_nal();
int streamer_write_h264_header(unsigned char *sps_dec, size_t sps_len,
                               unsigned char *pps_dec, size_t pps_len);
#endif
