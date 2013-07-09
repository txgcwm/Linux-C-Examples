/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/* headers required by common glibc calls */
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>


/* Duda headers and definition */
#include "webservice.h"
#include "channels.h"
#include "network.h"
#include "html.h"

DUDA_REGISTER("Duda HTTP Service", "H264 Streamer");

/* List channels available */
void cb_list(duda_request_t *dr)
{
    /*
     * Channels are registered into the filesystem under
     * the '/tmp' directory, format is the following:
     *
     * - Each channel must have two files:
     *   - Metadata file (stream header SPS PSP): channel_name.h264st.meta
     *   - Unix socket file: channel_name.h264st.sock
     */

    unsigned int offset;
    DIR *dir;
    struct dirent *ent;

    dir = opendir(CH_ROOT);
    if (!dir) {
        response->http_status(dr, 404);
        response->end(dr, NULL);
    }

    response->printf(dr, HTML_CHANNEL_HEADER);

    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') {
            continue;
        }

        /* Look just for regular files and socket */
        if (ent->d_type != DT_REG && ent->d_type != DT_SOCK) {
            continue;
        }

        if (strlen(ent->d_name) <= sizeof(CH_SOCK)) {
            continue;
        }

        offset = (unsigned int) (strlen(ent->d_name) - sizeof(CH_SOCK) + 1);
        if (strncmp(ent->d_name + offset, CH_SOCK, sizeof(CH_SOCK)) != 0) {
            continue;
        }

        response->printf(dr, "Channel: %s<br>\n", ent->d_name);
    }

    closedir(dir);

    response->printf(dr, HTML_CHANNEL_FOOTER);
    response->http_status(dr, 200);
    response->http_header(dr, "Content-Type: text/html");
    response->end(dr, NULL);
}

/* Connect to a streaming */
void cb_play(duda_request_t *dr)
{
    int s;
    int fd;
    int len;
    int size = 64;
    int chunk_len;
    int raw_size = 65536*4;
    char raw[raw_size];
    char *h264_header;
    char *channel;
    char *file_sock;
    char *file_meta;
    char chunk[size];
    struct stat st;
    const char *base_url = "/h264streamer/play/";

    /*
     * Get channel name
     *
     */
    s = (dr->sr->uri.len - strlen(base_url));

    if (s < 1) {
        response->http_status(dr, 404);
        response->end(dr, NULL);
    }

    channel = monkey->mem_alloc(s + 1);
    strncpy(channel, dr->sr->uri.data + strlen(base_url), s);
    channel[s] = '\0';

    /* file sock */
    s = strlen(CH_ROOT) + strlen(CH_SOCK) + strlen(channel) + 1;
    file_sock = malloc(s);
    snprintf(file_sock, s, "%s%s%s", CH_ROOT, channel, CH_SOCK);

    /* file meta */
    file_meta = malloc(s);
    snprintf(file_meta, s, "%s%s%s", CH_ROOT, channel, CH_META);

    /* validate meta data file */
    if (stat(file_meta, &st) != 0) {
        response->http_status(dr, 400);
        response->printf(dr, "Invalid channel");
        response->end(dr, NULL);
    }

    /* read meta header */
    h264_header = malloc(st.st_size);
    fd = open(file_meta, O_RDONLY);
    read(fd, h264_header, st.st_size);
    close(fd);

    /* response headers */
    response->http_status(dr, 200);
    response->http_content_length(dr, -1);
    response->http_header(dr, HTTP_CONTENT_TYPE_H264);
    response->http_header(dr, HTTP_CHUNKED_TE);
    response->send_headers(dr);

    /* meta */
    len = snprintf(chunk, size, "%X\r\n", (int) st.st_size);
    response->print(dr, chunk, len);
    response->print(dr, h264_header, st.st_size);
    response->flush(dr);

    //send(dr->cs->socket, chunk, len, 0);
    //send(dr->cs->socket, h264_header, st.st_size, 0);

    /* Connect to the decoded h264 video stream */
    int stream = -1;
    while (stream == -1) {
        stream = network_connect(file_sock);
    }

    printf("Connected to stream...\n");
    while (1) {
        memset(raw, '\0', sizeof(raw));
        len = recv(stream, raw, raw_size, 0);
        if (len == -1) {
            sleep(0.2);
            continue;
        }
        printf("recv=%i\n", len);

        chunk_len = snprintf(chunk, size, "%X\r\n", len);

        /*
        response->print(dr, chunk, chunk_len);
        response->print(dr, raw, len);
        response->flush(dr);
        */
        int r;

        r = send(dr->cs->socket, chunk, chunk_len, 0);
        printf("print chunk ret = %i\n", r);
        if (r == -1) {
            perror("send");
        }

        r = send(dr->cs->socket, raw, len, 0);
        printf("print raw   ret = %i\n", r);
        if (r == -1) {
            perror("send");
        }

        continue;


        r = response->print(dr, chunk, chunk_len);
        printf("print chunk ret = %i\n", r);


        r = response->print(dr, raw, len);
        printf("print chunk ret = %i\n", r);
        len = response->flush(dr);
        printf(" BYTES SENT: %i\n", len);
    }
    //response->end(dr, NULL);
}

int duda_main()
{
    map->static_add("/channels/", "cb_list");
    map->static_add("/play", "cb_play");

    return 0;
}
