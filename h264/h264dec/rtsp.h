/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  RTSP Client
 *  -----------
 *  Written by Eduardo Silva P. <edsiper@gmail.com>
 */

#include <stdint.h>

#ifndef RTSP_H
#define RTSP_H

/*
 * This struct represents a session established with the RTSP server
 * so the data contained is used for later PLAY commands
 */
struct rtsp_session {
  int socket;
  char *stream;

  /* RTSP protocol stuff */
    unsigned int packetization; /* Packetization mode from SDP data */
    unsigned int cport_from;    /* client_port from */
    unsigned int cport_to;      /* client port to   */
    unsigned int sport_from;    /* server port from */
    unsigned int sport_to;      /* server port to   */
    unsigned long session;      /* session ID       */
};

/* Last Sender Report timestamp (middle 32 bits) */
//uint32_t rtcp_last_sr_ts;

#define VERSION           "0.1"
#define PROTOCOL_PREFIX   "rtsp://"
#define RTSP_PORT         554
#define RTSP_CLIENT_PORT  9500
#define RTSP_RESPONSE     "RTSP/1.0 "
#define CMD_OPTIONS       "OPTIONS rtsp://%s:%lu RTSP/1.0\r\nCSeq: %i\r\n\r\n"
#define CMD_DESCRIBE      "DESCRIBE %s RTSP/1.0\r\nCSeq: %i\r\nAccept: application/sdp\r\n\r\n"

//#define CMD_SETUP         "SETUP %s/trackID=1 RTSP/1.0\r\nCSeq: %i\r\nTransport: RTP/AVP;unicast;client_port=%i-%i\r\n\r\n"

#define CMD_SETUP         "SETUP %s/trackID=1 RTSP/1.0\r\nCSeq: %i\r\nTransport: RTP/AVP/TCP;interleaved=0-1;\r\n\r\n"



#define CMD_PLAY          "PLAY %s RTSP/1.0\r\nCSeq: %i\r\nSession: %lu\r\nRange: npt=0.00-\r\n\r\n"

/* Transport header constants */
#define SETUP_SESSION      "Session: "
#define SETUP_TRNS_CLIENT  "client_port="
#define SETUP_TRNS_SERVER  "server_port="

#endif
