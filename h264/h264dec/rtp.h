/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  RTSP Client
 *  -----------
 *  Written by Eduardo Silva P. <edsiper@gmail.com>
 */

#include <stdint.h>

#ifndef RTP_H
#define RTP_H

/*
 * Struct taken from RFC 3550
 * --------------------------
 * http://www.ietf.org/rfc/rfc3550.txt
 */

struct rtp_header {
    unsigned int version:2;     /* protocol version */
    unsigned int padding:1;     /* padding flag */
    unsigned int extension:1;   /* header extension flag */
    unsigned int cc:4;          /* CSRC count */
    unsigned int marker:1;      /* marker bit */
    unsigned int pt:7;          /* payload type */
    uint16_t seq:16;            /* sequence number */
    uint32_t ts;                /* timestamp */
    uint32_t ssrc;              /* synchronization source */
    uint32_t csrc[1];           /* optional CSRC list */
};

struct rtp_stats {
    uint16_t first_seq;         /* first sequence                   */
    uint16_t highest_seq;       /* highest sequence                 */
    uint16_t rtp_received;      /* RTP sequence number received     */
    uint32_t rtp_identifier;    /* source identifier                */
    uint32_t rtp_ts;            /* RTP timestamp                    */
    uint32_t rtp_cum_lost;       /* RTP cumulative packet lost       */
    uint32_t rtp_expected_prior;/* RTP expected prior               */
    uint32_t rtp_received_prior;/* RTP received prior               */
    uint32_t transit;           /* Transit time. RFC3550 A.8        */
    uint32_t jitter;            /* Jitter                           */
    uint32_t lst;
    uint32_t last_dlsr;         /* Last DLSR                        */
    uint32_t last_rcv_SR_ts;    /* Last arrival in RTP format       */
    uint32_t delay_snc_last_SR; /* Delay sinde last SR              */
    struct timeval
    last_rcv_SR_time;           /* Last SR arrival                  */
    struct timeval
    last_rcv_time;
    double rtt_frac;
} rtp_st;

#define RTP_FREQ    90000
#define RTP_SPROP   "sprop-parameter-sets="

/* Enumeration of H.264 NAL unit types */
enum {
    NAL_TYPE_UNDEFINED = 0,
    NAL_TYPE_SINGLE_NAL_MIN	= 1,
    NAL_TYPE_SINGLE_NAL_MAX	= 23,
    NAL_TYPE_STAP_A		= 24,
    NAL_TYPE_FU_A		= 28,
};

uint64_t rtp_timeval_to_ntp(const struct timeval *tv);
void rtp_stats_update(struct rtp_header *rtp_h);
void rtp_stats_reset();
void rtp_stats_print();
unsigned int rtp_parse(unsigned char *raw, unsigned int size);

#endif
