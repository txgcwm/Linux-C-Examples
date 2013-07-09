/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  RTSP Client
 *  -----------
 *  Written by Eduardo Silva P. <edsiper@gmail.com>
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

/* networking I/O */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

/* local headers */
#include "network.h"
#include "streamer.h"
#include "base64.h"
#include "rtsp.h"
#include "utils.h"
#include "rtcp.h"
#include "rtp.h"
#include "h264dec.h"

void rtsp_cseq_inc()
{
    rtsp_cseq++;
}

void rtsp_header(int fd, int channel, uint16_t length)
{
    uint8_t tmp_8;
    uint16_t tmp_16;

    tmp_8 = 0x24;      /* RTSP magic number */
    send(fd, &tmp_8, 1, 0);

    tmp_8 = channel;   /* channel */
    send(fd, &tmp_8, 1, 0);

    tmp_16 = length;
    net_send16(fd, tmp_16);
}

void rtsp_rtcp_reports(int fd)
{
    //    net_sock_cork(fd, 1);
    rtsp_header(fd, 1, 74);

    /* report 1 */
    rtcp_receiver_report(fd);       /* 32 bytes */
    rtcp_receiver_desc(fd);         /* 17 bytes */

    usleep(200);
    /* report 0 */
    //rtcp_receiver_report_zero(fd);  /*  8 bytes */
    //rtcp_receiver_desc(fd);         /* 17 bytes */

    //net_sock_cork(fd, 0);

    printf("================RTCP SENT!=================\n");
}



/*
 * Returns the RTSP status code from the response, if an error occurred it
 * allocates a memory buffer and store the error message on 'error' variable
 */
int rtsp_response_status(char *response, char **error)
{
    int size = 256;
    int err_size;
    int offset = sizeof(RTSP_RESPONSE) - 1;
    char buf[8];
    char *sep;
    char *eol;
    *error = NULL;

    if (strncmp(response, RTSP_RESPONSE, offset) != 0) {
        *error = malloc(size);
        snprintf(*error, size, "Invalid RTSP response format");
        return -1;
    }

    sep = strchr(response + offset, ' ');
    if (!sep) {
        *error = malloc(size);
        snprintf(*error, size, "Invalid RTSP response format");
        return -1;
    }

    memset(buf, '\0', sizeof(buf));
    strncpy(buf, response + offset, sep - response - offset);

    eol = strchr(response, '\r');
    err_size = (eol - response) - offset - 1 - strlen(buf);
    *error = malloc(err_size + 1);
    strncpy(*error, response + offset + 1 + strlen(buf), err_size);

    return atoi(buf);
}

int rtsp_cmd_options(int sock, char *stream)
{
    int n;
    int ret = 0;
    int status;
    int size = 4096;
    char *err;
    char buf[size];

    RTSP_INFO("OPTIONS: command\n");

    memset(buf, '\0', sizeof(buf));
    n = snprintf(buf, size, CMD_OPTIONS, stream_host, stream_port, rtsp_cseq);
    DEBUG_REQ(buf);
    n = send(sock, buf, n, 0);

    RTSP_INFO("OPTIONS: request sent\n");

    memset(buf, '\0', sizeof(buf));
    n = recv(sock, buf, size - 1, 0);
    if (n <= 0) {
        printf("Error: Server did not respond properly, closing...");
        close(sock);
        exit(EXIT_FAILURE);
    }

    status = rtsp_response_status(buf, &err);
    if (status == 200) {
        RTSP_INFO("OPTIONS: response status %i (%i bytes)\n", status, n);
    }
    else {
        RTSP_INFO("OPTIONS: response status %i: %s\n", status, err);
        ret = -1;
    }

    DEBUG_RES(buf);
    rtsp_cseq_inc();

    return ret;
}

int rtsp_cmd_describe(int sock, char *stream, char **sprop)
{
    int n;
    int ret = 0;
    int status;
    int size = 4096;
    char *p, *end;
    char *err;
    char buf[size];

    RTSP_INFO("DESCRIBE: command\n");

    memset(buf, '\0', sizeof(buf));
    n = snprintf(buf, size, CMD_DESCRIBE, stream, rtsp_cseq);
    DEBUG_REQ(buf);
    n = send(sock, buf, n, 0);

    RTSP_INFO("DESCRIBE: request sent\n");

    memset(buf, '\0', sizeof(buf));
    n = recv(sock, buf, size - 1, 0);
    if (n <= 0) {
        printf("Error: Server did not respond properly, closing...");
        close(sock);
        exit(EXIT_FAILURE);
    }

    status = rtsp_response_status(buf, &err);
    if (status == 200) {
        RTSP_INFO("DESCRIBE: response status %i (%i bytes)\n", status, n);
    }
    else {
        RTSP_INFO("DESCRIBE: response status %i: %s\n", status, err);
        ret = -1;
    }

    DEBUG_RES("%s\n", buf);
    rtsp_cseq_inc();

    /* set the DSP information */
    p = strstr(buf, "\r\n\r\n");
    if (!p) {
        return -1;
    }

    /* Create buffer for DSP */
    dsp = malloc(n + 1);
    memset(dsp, '\0', n + 1);
    strcpy(dsp, p + 4);

    /* sprop-parameter-sets key */
    p = strstr(dsp, RTP_SPROP);
    if (!p) {
        return -1;
    }

    end = strchr(p, '\r');
    if (!end) {
        return -1;
    }

    int prop_size = (end - p) - sizeof(RTP_SPROP) + 1;
    *sprop = malloc(prop_size + 1);
    memcpy(*sprop, p + sizeof(RTP_SPROP) - 1, prop_size);

    return ret;
}

int rtsp_cmd_setup(int sock, char *stream, struct rtsp_session *session)
{
    int n;
    int ret = 0;
    int status;
    int field_size = 16;
    int size = 4096;
    int client_port_from = -1;
    int client_port_to = -1;
    int server_port_from = -1;
    int server_port_to = -1;
    unsigned long session_id;
    char *p;
    char *sep;
    char *err;
    char buf[size];
    char field[field_size];

    RTSP_INFO("SETUP: command\n");

    memset(buf, '\0', sizeof(buf));
    //n = snprintf(buf, size, CMD_SETUP, stream, rtsp_cseq, client_port, client_port + 1);
    n = snprintf(buf, size, CMD_SETUP, stream, rtsp_cseq);

    DEBUG_REQ(buf);
    n = send(sock, buf, n, 0);

    RTSP_INFO("SETUP: request sent\n");

    memset(buf, '\0', sizeof(buf));
    n = recv(sock, buf, size - 1, 0);
    if (n <= 0) {
        printf("Error: Server did not respond properly, closing...");
        close(sock);
        exit(EXIT_FAILURE);
    }

    status = rtsp_response_status(buf, &err);
    if (status == 200) {
        RTSP_INFO("SETUP: response status %i (%i bytes)\n", status, n);

        /* Fill session data */
        p = strstr(buf, "Transport: ");
        if (!p) {
            RTSP_INFO("SETUP: Error, Transport header not found\n");
            DEBUG_RES(buf);
            return -1;
        }

        /*
         * Commenting out this code, this part was written to support
         * RTP connection over UDP socket and determinate the server
         * ports to connect.
         *
         * By now the program is using TCP in Intervealed mode, so no
         * extra ports are required.

        buf_client_port = strstr(p, SETUP_TRNS_CLIENT);
        buf_server_port = strstr(p, SETUP_TRNS_SERVER);

        if (!buf_client_port || !buf_server_port) {
            RTSP_INFO("SETUP: Error, ports not defined in Transport header\n");
            DEBUG_RES(buf);
            return -1;
        }

         client_port from
        sep = strchr(buf_client_port + sizeof(SETUP_TRNS_CLIENT) - 1, '-');
        if (!sep) {
            RTSP_INFO("SETUP: client_port have an invalid format\n");
            DEBUG_RES(buf);
            return -1;
        }

        memset(field, '\0', sizeof(field));
        strncpy(field,
                buf_client_port + sizeof(SETUP_TRNS_CLIENT) - 1,
                sep - buf_client_port - sizeof(SETUP_TRNS_CLIENT) + 1);

        client_port_from = atoi(field);

        client_port to
        p = strchr(sep, ';');
        if (!p) {
            p = strchr(sep, '\r');
            if (!p) {
                RTSP_INFO("SETUP: client_port have an invalid format\n");
                DEBUG_RES(buf);
                return -1;
            }
        }

        memset(field, '\0', sizeof(field));
        strncpy(field, sep + 1, p - sep - 1);
        client_port_to = atoi(field);

         server_port from
        sep = strchr(buf_server_port + sizeof(SETUP_TRNS_SERVER) - 1, '-');
        if (!sep) {
            RTSP_INFO("SETUP: server_port have an invalid format\n");
            DEBUG_RES(buf);
            return -1;
        }


        memset(field, '\0', sizeof(field));
        strncpy(field,
                buf_server_port + sizeof(SETUP_TRNS_SERVER) - 1,
                sep - buf_server_port - sizeof(SETUP_TRNS_SERVER) + 1);

        server_port_from = atoi(field);

         server_port to
        p = strchr(sep, ';');
        if (!p) {
            p = strchr(sep, '\r');
            if (!p) {
                RTSP_INFO("SETUP: server_port have an invalid format\n");
                DEBUG_RES(buf);
                return -1;
            }
        }

        memset(field, '\0', sizeof(field));
        strncpy(field, sep + 1, p - sep - 1);
        server_port_to = atoi(field);
        */

        /* Session ID */
        p = strstr(buf, SETUP_SESSION);
        if (!p) {
            RTSP_INFO("SETUP: Session header not found\n");
            DEBUG_RES(buf);
            return -1;
        }

        sep = strchr(p, '\r');
        memset(field, '\0', sizeof(field));
        strncpy(field, p + sizeof(SETUP_SESSION) - 1, sep - p - sizeof(SETUP_SESSION) + 1);
        session_id = atol(field);
    }
    else {
        RTSP_INFO("SETUP: response status %i: %s\n", status, err);
        DEBUG_RES(buf);
        return -1;
    }

    /* Fill session data */
    session->packetization = 1; /* FIXME: project specific value */
    session->cport_from = client_port_from;
    session->cport_to   = client_port_to;
    session->sport_from = server_port_from;
    session->sport_to   = server_port_to;
    session->session    = session_id;

    DEBUG_RES(buf);
    rtsp_cseq_inc();
    return ret;
}

int rtsp_cmd_play(int sock, char *stream, unsigned long session)
{
    int n;
    int ret = 0;
    int status;
    int size = 4096;
    char *err;
    char buf[size];

    RTSP_INFO("PLAY: command\n");

    memset(buf, '\0', sizeof(buf));
    n = snprintf(buf, size, CMD_PLAY, stream, rtsp_cseq, session);
    DEBUG_REQ(buf);
    n = send(sock, buf, n, 0);

    RTSP_INFO("PLAY: request sent\n");

    memset(buf, '\0', sizeof(buf));

    n = recv(sock, buf, size - 1, 0);
    if (n <= 0) {
        printf("Error: Server did not respond properly, closing...");
        close(sock);
        exit(EXIT_FAILURE);
    }

    status = rtsp_response_status(buf, &err);
    if (status == 200) {
        RTSP_INFO("PLAY: response status %i (%i bytes)\n", status, n);
    }
    else {
        RTSP_INFO("PLAY: response status %i: %s\n", status, err);
        ret = -1;
    }

    DEBUG_RES(buf);
    rtsp_cseq_inc();
    return ret;
}


int rtsp_connect(char *stream)
{
    char *host;
    char *sep;
    char buf[8];
    int len;
    int pos;
    int offset = sizeof(PROTOCOL_PREFIX) - 1;

    /* Lookup the host address */
    if (!(sep = strchr(stream + offset, ':'))) {
        sep = strchr(stream + offset, '/');
    }

    if (!sep) {
        printf("Error: Invalid stream address '%s'", stream);
        exit(EXIT_FAILURE);
    }
    len = (sep - stream) - offset;
    host = malloc(len + 1);
    strncpy(host, stream + offset, len);
    host[len] = '\0';

    /* Lookup TCP port if specified */
    sep = strchr(stream + offset + 1, ':');
    if (sep) {
        pos = (sep - stream) + 1;
        sep = strchr(stream + pos, '/');
        if (!sep) {
            printf("Error: Invalid stream address '%s'\n", stream);
            exit(EXIT_FAILURE);
        }

        len = (sep - stream) - pos;
        if (len > sizeof(buf) - 1) {
            printf("Error: Invalid TCP port in stream address '%s'\n", stream);
            exit(EXIT_FAILURE);
        }

        memset(buf, '\0', sizeof(buf));
        strncpy(buf, stream + pos, len);
        stream_port = atol(buf);

    }

    stream_host = host;
    RTSP_INFO("Connecting to host '%s' port %lu...\n", stream_host, stream_port);
    return net_tcp_connect(stream_host, stream_port);
}

int rtsp_loop()
{
     int fd;
     int ret;
     struct rtsp_session rtsp_s;
     pid_t streamer_worker;

     /* Connect to server */
     fd = rtsp_connect(opt_stream);
     if (fd <= 0) {
         return -1;
     }

     ret = rtsp_cmd_options(fd, opt_stream);
     if (ret != 0) {
         return -1;
     }

     char *params;
     ret = rtsp_cmd_describe(fd, opt_stream, &params);
     if (ret != 0) {
         printf("Error: Could not send DESCRIBE command to RTSP server\n");
         return -1;
     }

     rtsp_s.socket = fd;
     rtsp_s.stream = strdup(opt_stream);

     rtsp_cmd_setup(fd, opt_stream, &rtsp_s);
     rtsp_cmd_play(fd, opt_stream, rtsp_s.session);

     rtp_stats_reset();
     rtp_st.rtp_identifier = rtsp_s.session;
     //rtsp_rtcp_reports(fd);

     /* H264 Parameters, taken from the SDP output */
     int p_size;
     char *sep;
     char *sps;
     char *pps;
     unsigned char *sps_dec;
     unsigned char *pps_dec;
     size_t sps_len;
     size_t pps_len;

     /* SPS */
     sep = strchr(params, ',');
     p_size = (sep - params);
     sps = malloc(p_size + 1);
     memset(sps, '\0', p_size + 1);
     memcpy(sps, params, p_size);

     /* PPS */
     p_size = (strlen(params) - p_size);
     pps = malloc(p_size + 1);
     memset(pps, '\0', p_size + 1);
     memcpy(pps, sep + 1, p_size);

     /* Decode each parameter */
     sps_dec = base64_decode((const unsigned char *) sps, strlen(sps), &sps_len);
     pps_dec = base64_decode((const unsigned char *) pps, strlen(pps), &pps_len);

     free(sps);
     free(pps);

     int channel;
     int r;
     int max_buf_size = 1000000;

     unsigned char raw[max_buf_size];
     unsigned char raw_tmp[max_buf_size];
     unsigned int raw_length;
     unsigned int raw_offset = 0;
     unsigned int rtp_length;

     /* open debug file */
     if (stream_dump) {
         stream_fs_fd = open(stream_dump, O_CREAT|O_WRONLY|O_TRUNC, 0666);
     }
     else {
         stream_fs_fd = -1;
     }

     /* write H264 header */
     streamer_write_h264_header(sps_dec, sps_len, pps_dec, pps_len);

     /* Create unix named pipe */
     stream_sock = streamer_prepare(opt_name, sps_dec, sps_len, pps_dec, pps_len);
     if (stream_sock <= 0) {
         printf("Error: could not create unix socket\n\n");
         exit(EXIT_FAILURE);
     }

     /* Local pipe */
     streamer_pipe_init(stream_pipe);
     streamer_worker = streamer_loop(stream_sock);

     printf("Streaming will start shortly...\n");

     /* Set recv timeout for fd */
     struct timeval tv;
     tv.tv_sec = 15;  /* 30 Secs Timeout */
     tv.tv_usec = 0;  // Not init'ing this can cause strange errors
     setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

     int send_rtcp = 0;
     rtcp_worker(fd);
     while (1) {
         raw_offset = 0;
         raw_length = 0;
         memset(raw, '\0', sizeof(raw));

     read_pending:

         /* is buffer full ? */
         if (raw_length >= max_buf_size) {
             printf(">> RESETTING\n");

             if (raw_offset < raw_length - 1) {
                 int bytes = raw_length - raw_offset;

                 memset(raw_tmp, '\0', sizeof(raw_tmp));
                 memcpy(raw_tmp, raw + raw_offset, bytes);

                 memset(raw, '\0', sizeof(raw));
                 memcpy(raw, raw_tmp, bytes);

                printf("   Move:  %i\n", raw_length - raw_offset);
                raw_length = bytes;
                raw_offset = 0;
                if (raw[raw_offset] != 0x24) {
                    printf("MASTER CORRUPTION\n");
                }

            }
            else {
                raw_length = 0;
                memset(raw, '\0', sizeof(raw));
            }
         }

         /* read incoming data */
         printf(">> RECV: max %i bytes\n", max_buf_size - raw_length);
         r = recv(fd, raw + raw_length, max_buf_size - raw_length, 0);
         printf(">> READ: %i (up to %i)\n", r, max_buf_size - raw_length);
         fflush(stdout);

         if (r <= 0) {
             if (errno == EAGAIN) {
                 //printf("Socket timeout, send out RTCP packets\n");
                 //goto read_pending;
             }

             rtp_stats_print();
             printf(">> RTSP: Server closed connection!\n");

            /* cleanup */
            if (stream_fs_fd > 0) {
                close(stream_fs_fd);
            }

            close(fd);
            close(stream_sock);
            close(stream_pipe[0]);
            close(stream_pipe[1]);
            exit(1);
            return -1;
         }

         raw_length += r;

        /* parse all data in our buffer */
        while (raw_offset < raw_length) {

            /* RTSP Interleaved header */
            if (raw[raw_offset] == 0x24) {
                channel = raw[raw_offset + 1];
                rtp_length  = (raw[raw_offset + 2] << 8 | raw[raw_offset + 3]);

                printf(">> RTSP Interleaved (offset = %i/%i)\n",
                       raw_offset, raw_length);
                printf("   Magic         : 0x24\n");
                printf("   Channel       : %i\n", channel);
                printf("   Payload Length: %i\n", rtp_length);

                if (raw_length > max_buf_size) {
                    printf("Error exception: raw_length = %i ; max_buf_size = %i\n",
                           raw_length, max_buf_size);
                    exit(EXIT_FAILURE);
                }

                /* RTSP header is 4 bytes, update the offset */
                raw_offset += 4;

                /* If no payload exists, continue with next bytes */
                if (rtp_length == 0) {
                    raw_offset -= 4;
                    goto read_pending;
                    continue;
                }

                if (rtp_length > (raw_length - raw_offset)) {
                    raw_offset -= 4;
                    printf("   ** Pending    : %u bytes\n",
                           rtp_length - (raw_length - raw_offset));
                          //[CH %i] PENDING: RTP_LENGTH=%i ; RAW_LENGTH=%i; RAW_OFFSET=%i\n",
                          // channel, rtp_length, raw_length, raw_offset);
                    if (send_rtcp == 1){
                        rtsp_rtcp_reports(fd);
                        send_rtcp = 0;
                    }

                    goto read_pending;
                }

                if (rtp_st.rtp_received % 20) {
                    rtsp_rtcp_reports(fd);
                }

                /* RTCP data */
                if (channel >= 1) {
                    int idx;
                    int rtcp_count;
                    int size_RTCP_SR = 32;
                    int size_RTCP_SDES = 17;
                    int size_RTCP = 0;

                    struct rtcp_pkg *rtcp;

                    /* Decode RTCP packet(s) */
                    rtcp = rtcp_decode(raw + raw_offset, rtp_length, &rtcp_count);

                    if (rtcp_count >= 1 && rtcp[0].type == RTCP_SR) {
                        send_rtcp = 1;
                        //rtsp_rtcp_reports(fd);
                    }

                    /* For each RTCP packet, send a reply
                    for (idx = 0; idx < rtcp_count; idx++) {
                        if (rtcp[idx].type == RTCP_SR) {
                            size_RTCP += size_RTCP_SR;
                        }
                        else if (rtcp[idx].type == RTCP_SDES) {
                            size_RTCP += size_RTCP_SDES;
                        }
                    }

                    net_sock_cork(fd, 1);  INVALID
                    rtsp_header(fd, 1, size_RTCP);

                    for (idx = 0; idx < rtcp_count; idx++) {
                         sender report, send a receiver report
                        if (rtcp[idx].type == RTCP_SR) {
                            rtcp_receiver_report(fd);
                        }
                        else if (rtcp[idx].type == RTCP_SDES) {
                            rtcp_receiver_desc(fd);
                        }
                    }
                    net_sock_cork(fd, 0); INVALID
                    */
                    raw_offset += rtp_length;
                    free(rtcp);
                    continue;
                }

                if (rtp_length == 0) {
                    continue;
                }

                /*
                 * Channel 0
                 * ---------
                 * If the channel is zero, the payload should contain RTP data,
                 * we need to identify the RTP header fields so later we can
                 * proceed to extract the H264 information.
                 */
                int offset;
                offset = rtp_parse(raw + raw_offset, rtp_length);
                if (offset <= 0) {
                    raw_offset += rtp_length;
                }
                else {
                    raw_offset += offset;
                }

                if (send_rtcp == 1) {
                    //rtsp_rtcp_reports(fd);
                    send_rtcp = 0;
                }

                continue;
            }
            raw_offset++;
            continue;
        }
        continue;
    }
     close(stream_fs_fd);

    return 0;
}
