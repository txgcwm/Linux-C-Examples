/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  RTSP Client
 *  -----------
 *  Written by Eduardo Silva P. <edsiper@gmail.com>
 */

/* generic headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>

/* local headers */
#include "buffer.h"
#include "rtsp.h"
#include "rtcp.h"
#include "rtp.h"
#include "network.h"
#include "utils.h"
#include "streamer.h"

int rtp_connect(char *stream)
{
    char *host;
    char *sep;
    int len;
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

    RTP_INFO("Connecting to host '%s' port %i...\n", host, 0);

    return net_udp_connect(host, 0);
}

unsigned int rtp_parse(unsigned char *raw, unsigned int size)
{
    unsigned int raw_offset = 0;
    unsigned int rtp_length = size;
    unsigned int paysize;
    unsigned char payload[MAX_BUF_SIZE];
    struct rtp_header rtp_h;

    rtp_h.version = raw[raw_offset] >> 6;
    rtp_h.padding = CHECK_BIT(raw[raw_offset], 5);
    rtp_h.extension = CHECK_BIT(raw[raw_offset], 4);
    rtp_h.cc = raw[raw_offset] & 0xFF;

    /* next byte */
    raw_offset++;

    rtp_h.marker = CHECK_BIT(raw[raw_offset], 8);
    rtp_h.pt     = raw[raw_offset] & 0x7f;

    /* next byte */
    raw_offset++;

    /* Sequence number */
    rtp_h.seq = raw[raw_offset] * 256 + raw[raw_offset + 1];
    raw_offset += 2;

    /* time stamp */
    rtp_h.ts = \
        (raw[raw_offset    ] << 24) |
        (raw[raw_offset + 1] << 16) |
        (raw[raw_offset + 2] <<  8) |
        (raw[raw_offset + 3]);
    raw_offset += 4;

    /* ssrc / source identifier */
    rtp_h.ssrc = \
        (raw[raw_offset    ] << 24) |
        (raw[raw_offset + 1] << 16) |
        (raw[raw_offset + 2] <<  8) |
        (raw[raw_offset + 3]);
    raw_offset += 4;
    rtp_st.rtp_identifier = rtp_h.ssrc;

    /* Payload size */
    paysize = (rtp_length - raw_offset);

    memset(payload, '\0', sizeof(payload));
    memcpy(&payload, raw + raw_offset, paysize);

    /*
     * A new RTP packet has arrived, we need to pass the rtp_h struct
     * to the stats/context updater
     */
    rtp_stats_update(&rtp_h);

    /* Display RTP header info */
    printf("   >> RTP\n");
    printf("      Version     : %i\n", rtp_h.version);
    printf("      Padding     : %i\n", rtp_h.padding);
    printf("      Extension   : %i\n", rtp_h.extension);
    printf("      CSRC Count  : %i\n", rtp_h.cc);
    printf("      Marker      : %i\n", rtp_h.marker);
    printf("      Payload Type: %i\n", rtp_h.pt);
    printf("      Sequence    : %i\n", rtp_h.seq);
    printf("      Timestamp   : %u\n", rtp_h.ts);
    printf("      Sync Source : %u\n", rtp_h.ssrc);

    /*
     * NAL, first byte header
     *
     *   +---------------+
     *   |0|1|2|3|4|5|6|7|
     *   +-+-+-+-+-+-+-+-+
     *   |F|NRI|  Type   |
     *   +---------------+
     */
    int nal_forbidden_zero = CHECK_BIT(payload[0], 7);
    int nal_nri  = (payload[0] & 0x60) >> 5;
    int nal_type = (payload[0] & 0x1F);

    printf("      >> NAL\n");
    printf("         Forbidden zero: %i\n", nal_forbidden_zero);
    printf("         NRI           : %i\n", nal_nri);
    printf("         Type          : %i\n", nal_type);

    /* Single NAL unit packet */
    if (nal_type >= NAL_TYPE_SINGLE_NAL_MIN &&
        nal_type <= NAL_TYPE_SINGLE_NAL_MAX) {

        /* Write NAL header */
        streamer_write_nal();

        /* Write NAL unit */
        streamer_write(payload, sizeof(paysize));
    }

    /*
     * Agregation packet - STAP-A
     * ------
     * http://www.ietf.org/rfc/rfc3984.txt
     *
     * 0                   1                   2                   3
     * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                          RTP Header                           |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |STAP-A NAL HDR |         NALU 1 Size           | NALU 1 HDR    |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                         NALU 1 Data                           |
     * :                                                               :
     * +               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |               | NALU 2 Size                   | NALU 2 HDR    |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                         NALU 2 Data                           |
     * :                                                               :
     * |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                               :...OPTIONAL RTP padding        |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     */
    else if (nal_type == NAL_TYPE_STAP_A) {
        uint8_t *q;
        uint16_t nalu_size;

        q = payload + 1;
        int nidx = 0;

        nidx = 0;
        while (nidx < paysize - 1) {
            /* write NAL header */
            streamer_write_nal();

            /* get NALU size */
            nalu_size = (q[nidx] << 8) | (q[nidx + 1]);
            printf("nidx = %i ; NAL size = %i ; RAW offset = %i\n",
                   nidx, nalu_size, raw_offset);
            nidx += 2;

            /* write NALU size */
            streamer_write(&nalu_size, 1);

            if (nalu_size == 0) {
                nidx++;
                continue;
            }

            /* write NALU data */
            streamer_write(q + nidx, nalu_size);
            nidx += nalu_size;
        }
    }
    else if (nal_type == NAL_TYPE_FU_A) {
        printf("         >> Fragmentation Unit\n");

        uint8_t *q;
        q = payload;

        uint8_t h264_start_bit = q[1] & 0x80;
        uint8_t h264_end_bit   = q[1] & 0x40;
        uint8_t h264_type      = q[1] & 0x1F;
        uint8_t h264_nri       = (q[0] & 0x60) >> 5;
        uint8_t h264_key       = (h264_nri << 5) | h264_type;

        if (h264_start_bit) {
            /* write NAL header */
            streamer_write_nal();

            /* write NAL unit code */
            streamer_write(&h264_key, sizeof(h264_key));
        }
        streamer_write(q + 2, paysize - 2);

        if (h264_end_bit) {
            /* nothing to do... */
        }
    }
    else if (nal_type == NAL_TYPE_UNDEFINED) {

    }
    else {
        printf("OTHER NAL!: %i\n", nal_type);
        raw_offset++;

    }
    raw_offset += paysize;

    if (rtp_h.seq > rtp_st.highest_seq) {
        rtp_st.highest_seq = rtp_h.seq;
    }

    rtp_stats_print();
    return raw_offset;
}

void rtp_rtp2tval(unsigned int ts, struct timeval *tv)
{
    tv->tv_sec  = (ts * RTP_FREQ);
    tv->tv_usec = ((((ts % RTP_FREQ) / (ts / 8000))) * 125);
}

uint64_t rtp_timeval_to_ntp(const struct timeval *tv)
{
    uint64_t msw;
    uint64_t lsw;

    /* 0x83AA7E80 is the number of seconds from 1900 to 1970 */
    msw = tv->tv_sec + 0x83AA7E80;
    lsw = (uint32_t)((double)tv->tv_usec*(double)(((uint64_t)1)<<32)*1.0e-6);

    return ((msw << 32) | lsw);
}

/*
uint32_t rtp_now()
{
    struct timeval tmp;

    gettimeofday(&tmp, NULL);
    tmp.tv_sec  -= rtp_st.ts_delta.tv_sec;
    tmp.tv_usec -= rtp_st.ts_delta.tv_usec;

    return rtp_tval2RTP(tmp);
    return rtp_tval2rtp(tmp.tv_sec, tmp.tv_usec);
}
*/
void rtp_stats_reset()
{
    memset(&rtp_st, '\0', sizeof(struct rtp_stats));
}

/* Every time a RTP packet arrive, update the stats */
void rtp_stats_update(struct rtp_header *rtp_h)
{
    uint32_t transit;
    int delta;
    struct timeval now;

    gettimeofday(&now, NULL);
    rtp_st.rtp_received++;

    /* Highest sequence */
    if (rtp_h->seq > rtp_st.highest_seq) {
        rtp_st.highest_seq = rtp_h->seq;
    }


    /* Update RTP timestamp */
    if (rtp_st.last_rcv_time.tv_sec == 0) {
        //rtp_st.rtp_ts = rtp_h->ts;
        rtp_st.first_seq = rtp_h->seq;
        //rtp_st.jitter = 0;
        //rtp_st.last_dlsr = 0;
        //rtp_st.rtp_cum_lost = 0;
        gettimeofday(&rtp_st.last_rcv_time, NULL);

        /* deltas
        int sec  = (rtp_h->ts / RTP_FREQ);
        int usec = (((rtp_h->ts % RTP_FREQ) / (RTP_FREQ / 8000))) * 125;
        rtp_st.ts_delta.tv_sec  = now.tv_sec - sec;
        rtp_st.ts_delta.tv_usec = now.tv_usec - usec;


        rtp_st.last_arrival = rtp_tval2rtp(rtp_st.ts_delta.tv_sec,
                                           rtp_st.ts_delta.tv_usec);
        rtp_st.last_arrival = rtp_tval2RTP(now);

    }
    else {*/
    }
        /* Jitter */
        transit = rtp_st.delay_snc_last_SR;
        //printf("TRANSIT!: %i\n", transit); exit(1);
        delta = transit - rtp_st.transit;
        rtp_st.transit = transit;
        if (delta < 0) {
            delta = -delta;
        }
        //printf("now = %i ; rtp = %i ; delta = %i\n",
        //       t, rtp_h->ts, delta);
        //rtp_st.jitter += delta - ((rtp_st.jitter + 8) >> 4);
        rtp_st.jitter += ((1.0/16.0) * ((double) delta - rtp_st.jitter));

        rtp_st.rtp_ts = rtp_h->ts;
        //}

    /* print the new stats */
    rtp_stats_print();
}

void rtp_stats_print()
{

    printf(">> RTP Stats\n");
    printf("   First Sequence  : %u\n", rtp_st.first_seq);
    printf("   Highest Sequence: %u\n", rtp_st.highest_seq);
    printf("   RTP Received    : %u\n", rtp_st.rtp_received);
    printf("   RTP Identifier  : %u\n", rtp_st.rtp_identifier);
    printf("   RTP Timestamp   : %u\n", rtp_st.rtp_ts);
    printf("   Jitter          : %u\n", rtp_st.jitter);
    printf("   Last DLSR       : %i\n", rtp_st.last_dlsr);
}
