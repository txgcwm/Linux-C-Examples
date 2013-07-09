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
#include <getopt.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

/* local headers */
#include "rtsp.h"
#include "streamer.h"
#include "h264dec.h"

void help(int status)
{
    printf("Usage: h264dec [-v] [-V] [-d FILENAME] -n CHANNEL_NAME -s rtsp://stream\n\n");
    printf("  -d, --dump=FILENAME      Dump H264 video data to a file\n");
    printf("  -s, --stream=URL         RTSP media stream address\n");
    printf("  -n, --name=CHANNEL_NAME  Name for local channel identifier\n");
	printf("  -h, --help               print this help\n");
	printf("  -v, --version            print version number\n");
	printf("  -V, --verbose            enable verbose mode\n");
	printf("\n");
	exit(status);
}

void banner()
{
    printf("H264Dec v%s\n\n", VERSION);
}

int main(int argc, char **argv)
{
     int opt;

     /* defaults */
     opt_stdout  = 0;
     opt_verbose = 0;
     opt_stream = NULL;
     opt_name = NULL;
     stream_port = RTSP_PORT;
     client_port = RTSP_CLIENT_PORT;
     stream_dump = NULL;

     static const struct option long_opts[] = {
         { "stdout",  no_argument      , NULL, 'o' },
         { "dump",    required_argument, NULL, 'd' },
         { "stream",  required_argument, NULL, 's' },
         { "port",    required_argument, NULL, 'p' },
         { "name",    required_argument, NULL, 'n' },
         { "version", no_argument,       NULL, 'v' },
         { "verbose", no_argument,       NULL, 'V' },
         { "help",    no_argument,       NULL, 'h' },
         { NULL, 0, NULL, 0 }
     };

     while ((opt = getopt_long(argc, argv, "od:s:p:n:vVh",
                               long_opts, NULL)) != -1) {
         switch (opt) {
         case 'o':
             opt_stdout = 1;
             break;
         case 'd':
             stream_dump = strdup(optarg);
             break;
         case 's':
             opt_stream = strdup(optarg);
             break;
         case 'p':
             client_port = atoi(optarg);
             break;
         case 'n':
             opt_name = strdup(optarg);
             break;
         case 'v':
             banner();
             exit(EXIT_SUCCESS);
         case 'V':
             opt_verbose = 1;
             break;
         case 'h':
             help(EXIT_SUCCESS);
         case '?':
             help(EXIT_FAILURE);
         }
     }


     if (!opt_stream || strncmp(opt_stream, PROTOCOL_PREFIX,
                                sizeof(PROTOCOL_PREFIX) - 1) != 0) {
         printf("Error: Invalid stream input.\n\n");
         help(EXIT_FAILURE);
     }

     if (!opt_name) {
         printf("Error: Local channel name not specified.\n\n");
         help(EXIT_FAILURE);
     }

     /* RTSP loop */
     while (1) {
         rtsp_loop();
         printf("[ERROR] RTSP Loop stopped, waiting 5 seconds...\n");
         sleep(5);
         exit(1);
     }


     return 0;
}
