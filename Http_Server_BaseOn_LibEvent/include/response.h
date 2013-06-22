/******************************************************************
 *
 *
 *
 *
 *
 *
 ******************************************************************/

#ifndef _RESPONSE_H_
#define _RESPONSE_H_

#include "http_server.h"

void dump_request_cb(struct evhttp_request *req, void *arg);

void send_document_cb(struct evhttp_request *req, void *arg);

#endif
