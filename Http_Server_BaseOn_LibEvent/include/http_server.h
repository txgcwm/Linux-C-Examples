/******************************************************************
 *
 *
 *
 *
 *
 *
 ******************************************************************/

#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#define DEFAULTIP 		"127.0.0.1"
#define DEFAULTPORT 	"80"
#define DEFAULTBACK 	"10"
#define DEFAULTDIR 		"/home"
#define DEFAULTLOG 		"/tmp/http-server.log"

#define MAXBUF 1024

typedef struct _HTTP_SERVER_ {
	char *host;
	char *port;
	char *rootdir;
	char *logdir;
	unsigned char daemon;
	char uri_root[128];
	char buffer[MAXBUF + 1];
} HttpServer, *pHttpServer;

#endif
