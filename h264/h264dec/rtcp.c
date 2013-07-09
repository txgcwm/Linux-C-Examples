/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  RTCP
 *  ----
 *  Written by Eduardo Silva P. <edsiper@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <unistd.h>

#include "rtp.h"
#include "rtcp.h"
#include "rtsp.h"
#include "network.h"

/* Calculate jitter bassed on stats */
static uint32_t rtcp_jitter()
{
    return rtp_st.jitter;
};

uint32_t rtcp_dlsr()
{
    struct timeval now;
    double delay;

    if (rtp_st.last_rcv_SR_ts == 0) {
        return 0;
    }

    gettimeofday(&now,NULL);
    delay= (now.tv_sec - rtp_st.last_rcv_SR_time.tv_sec) +
           ((now.tv_usec - rtp_st.last_rcv_SR_time.tv_usec)*1e-6);
    delay= (delay * 65536);
    rtp_st.delay_snc_last_SR = (uint32_t) delay;
    return rtp_st.delay_snc_last_SR;
}

static void compute_rtt(const struct timeval *now){
    uint64_t curntp = rtp_timeval_to_ntp(now);
    uint32_t approx_ntp = (curntp >> 16) & 0xFFFFFFFF;
    uint32_t last_sr_time = rtp_st.last_rcv_SR_ts;
    uint32_t sr_delay = rtp_st.delay_snc_last_SR;

    if (last_sr_time !=0 && sr_delay!=0){
        double rtt_frac = approx_ntp-last_sr_time-sr_delay;
        rtp_st.rtt_frac /= 65536.0;
    }
}

/*
 * Decode a RTSP payload and strip the RTCP frames, it returns an array of
 * rtcp_pkg struct and set the number of entries in count variable
 */
struct rtcp_pkg *rtcp_decode(unsigned char *payload,
                             unsigned long len, int *count)
{
    int i = 0;
    int start = 0;
    int idx = 0;
    struct timeval now;

    *count = 0;
    gettimeofday(&now, NULL);

    /*
     * We define a maximum of 16 RTCP packets to decode, rarely
     * we will have more than two.
     */
    struct rtcp_pkg *pk = malloc(sizeof(struct rtcp_pkg) * 16);

    while (i < len) {
        start = i;

        /* Decode RTCP */
        pk[idx].version   = (payload[i] >> 6);
        pk[idx].padding   = (payload[i] & 0x20) >> 5;
        pk[idx].extension = (payload[i] & 0x10) >> 4;
        pk[idx].ccrc      = (payload[i] & 0xF);

        i++;
        pk[idx].type      = (payload[i]);

        /* length */
        i++;
        pk[idx].length    = (payload[i] * 256);
        i++;
        pk[idx].length   += payload[i];

        if (debug_rtcp) {
            printf("RTCP Version  : %i\n", pk[idx].version);
            printf("     Padding  : %i\n", pk[idx].padding);
            printf("     Extension: %i\n", pk[idx].extension);
            printf("     CCRC     : %i\n", pk[idx].ccrc);
            printf("     Type     : %i\n", pk[idx].type);
            printf("     Length   : %i (%i bytes)\n",
                   pk[idx].length, (pk[idx].length + 1) * 4);
        }

        /* server report */
        if (pk[idx].type == RTCP_SR) {
            pk[idx].ssrc    = (
                               (payload[i + 4]) |
                               (payload[i + 3] <<  8) |
                               (payload[i + 2] << 16) |
                               (payload[i + 1] << 24)
                               );

            /* NTP time */
            pk[idx].ts_msw  =  (
                                (payload[i + 8]) |
                                (payload[i + 7] <<  8) |
                                (payload[i + 6] << 16) |
                                (payload[i + 5] << 24)
                                );

            pk[idx].ts_lsw  =  (
                                (payload[i + 12]) |
                                (payload[i + 11] <<  8) |
                                (payload[i + 10] << 16) |
                                (payload[i +  9] << 24)
                                );

            /* RTP timestamp */
            pk[idx].ts_rtp  =  (
                                (payload[i + 16]) |
                                (payload[i + 15] <<  8) |
                                (payload[i + 14] << 16) |
                                (payload[i + 13] << 24)
                                );

            pk[idx].sd_pk_c =  (
                                (payload[i + 20]) |
                                (payload[i + 19] <<  8) |
                                (payload[i + 18] << 16) |
                                (payload[i + 17] << 24)
                                );

            pk[idx].sd_pk_c =  (
                                (payload[i + 24]) |
                                (payload[i + 23] <<  8) |
                                (payload[i + 22] << 16) |
                                (payload[i + 21] << 24)
                                );
            i += 24;

            /* last SR */
            rtp_st.last_rcv_SR_ts = ((pk[idx].ts_msw & 0xFFFF) << 16) |
                                     (pk[idx].ts_lsw >> 16);

            rtp_st.last_rcv_SR_time.tv_sec  = now.tv_sec;
            rtp_st.last_rcv_SR_time.tv_usec = now.tv_usec;

            rtp_st.rtp_identifier = pk[idx].ssrc;

            if (debug_rtcp) {
                printf("     SSRC     : 0x%x (%u)\n",
                       pk[idx].ssrc, pk[idx].ssrc);
                printf("     TS MSW   : 0x%x (%u)\n",
                       pk[idx].ts_msw, pk[idx].ts_msw);
                printf("     TS LSW   : 0x%x (%u)\n",
                       pk[idx].ts_lsw, pk[idx].ts_lsw);
                printf("     TS RTP   : 0x%x (%u)\n",
                       pk[idx].ts_rtp, pk[idx].ts_rtp);
                printf("     SD PK CT : %u\n", pk[idx].sd_pk_c);
                printf("     SD OC CT : %u\n", pk[idx].sd_oc_c);
            }


        }
        /* source definition */
        else if (pk[idx].type == RTCP_SDES) {
            pk[idx].identifier = (
                                  (payload[i + 4]) |
                                  (payload[i + 3] <<  8) |
                                  (payload[i + 2] << 16) |
                                  (payload[i + 1] << 24)
                                  );
            i += 5;
            pk[idx].sdes_type = payload[i];

            i++;
            pk[idx].sdes_length = payload[i];

            /* we skip the source name, we dont need it */
            i += pk[idx].sdes_length;

            /* end ? */
            i++;
            pk[idx].sdes_type2 = payload[i];

            i++;
            if (debug_rtcp) {
                printf("     ID       : %u\n", pk[idx].identifier);
                printf("     Type     : %i\n", pk[idx].sdes_type);
                printf("     Length   : %i\n", pk[idx].sdes_length);
                printf("     Type 2   : %i\n", pk[idx].sdes_type2);
            }
       }

        if (debug_rtcp) {
            printf("     Len Check: ");
            if ( (i - start) / 4 != pk[idx].length) {
                printf("Error\n");

                printf("i     : %i\nstart : %i\n pk length: %i\n",
                       i, start, pk[idx].length);
                exit(1);
            }
            else {
                printf("OK\n");
            }
        }
        /* Discard packet */
        else {
            i += pk[idx].length;
        }
        i++;
        idx++;
    }

    *count = idx;
    return pk;
}


/* create a receiver report package */
int rtcp_receiver_report_zero(int fd)
{
    uint8_t  tmp_8;
    uint16_t tmp_16;
    uint32_t tmp_32;

    /* RTCP: version, padding, report count = 0; int = 128 ; hex = 0x80 */
    tmp_8 = 0x80;
    send(fd, &tmp_8, 1, 0);

    /* RTCP: packet type - receiver report */
    tmp_8 = RTCP_RR;
    send(fd, &tmp_8, 1, 0);

    /* RTCP: length */
    tmp_16 = 0x01;
    net_send16(fd, tmp_16);

    /* RTCP: sender SSRC */
    tmp_32 = RTCP_SSRC;
    net_send32(fd, tmp_32);

    return 0;
}

/* create a receiver report package */
int rtcp_receiver_report(int fd)
{
    uint8_t  tmp_8;
    uint16_t tmp_16;
    uint32_t tmp_32;
    struct timeval now;
    gettimeofday(&now, NULL);

    /* RTCP: version, padding, report count; int = 129 ; hex = 0x81 */
    tmp_8 = 0x81;
    send(fd, &tmp_8, 1, 0);

    /* RTCP: packet type - receiver report */
    tmp_8 = RTCP_RR;
    send(fd, &tmp_8, 1, 0);

    /* RTCP: length */
    tmp_16 = 0x07;
    net_send16(fd, tmp_16);

    /* RTCP: sender SSRC */
    tmp_32 = RTCP_SSRC;
    net_send32(fd, tmp_32);

    /* RTCP: Source 1: Identifier */
    net_send32(fd, rtp_st.rtp_identifier);

    /*
     * Calcs for expected and lost packets
     */
    uint32_t extended_max;
    uint32_t expected;
    extended_max = rtp_st.rtp_received + rtp_st.highest_seq;
    expected = extended_max - rtp_st.first_seq + 1;
    rtp_st.rtp_cum_lost = expected - rtp_st.rtp_received - 1;

    /* Fraction */
    uint32_t expected_interval;
    uint32_t received_interval;
    uint32_t lost_interval;
    uint8_t fraction;

    expected_interval = expected - rtp_st.rtp_expected_prior;
    rtp_st.rtp_expected_prior = expected;

    received_interval = rtp_st.rtp_received - rtp_st.rtp_received_prior;
    rtp_st.rtp_received_prior = rtp_st.rtp_received;
    lost_interval = expected_interval - received_interval;
    if (expected_interval == 0 || lost_interval <= 0) fraction = 0;
    else fraction = (lost_interval << 8) / expected_interval;

    /* RTCP: SSRC Contents: Fraction lost */
    //send(fd, &fraction, sizeof(fraction), 0);

    tmp_32 = 0;//(fraction << 24) | ((rtp_st.rtp_cum_lost ) & 0xFFFFFF);
    net_send32(fd, tmp_32);
    /* RTCP: SSRC Contents: Cumulative packet losts */
    //net_send24(fd, rtp_st.rtp_cum_lost);

    /* RTCP: SSRC Contents: Extended highest sequence */
    tmp_16 = 1;// FIXME!??? rtp_st.rtp_received;
    net_send16(fd, tmp_16);
    tmp_16 = rtp_st.highest_seq;
    net_send16(fd, tmp_16);

    /* RTCP: SSRC Contents: interarrival jitter */
    tmp_32 = rtcp_jitter();
    net_send32(fd, tmp_32);
    rtp_stats_print();

    /* RTCP: SSRC Contents: Last SR timestamp */
    tmp_32 = rtp_st.last_rcv_SR_ts;
    net_send32(fd, tmp_32);

    /* RTCP: SSRC Contents: Timestamp delay */
    uint32_t dlsr = rtcp_dlsr();
    net_send32(fd, dlsr);

    return 0;
}

int rtcp_receiver_desc(int fd)
{
    uint8_t  tmp_8;
    uint16_t tmp_16;
    uint32_t tmp_32;

    /* RTCP: version, padding, report count; int = 129 ; hex = 0x81 */
    tmp_8 = 0x81;
    send(fd, &tmp_8, 1, 0);

    /* RTCP: packet type - source description */
    tmp_8 = RTCP_SDES;
    send(fd, &tmp_8, 1, 0);

    /* RTCP: length */
    tmp_16 = 0x04; /* 11 bytes */
    net_send16(fd, tmp_16);

    /* RTCP: Source 1: Identifier */
    tmp_32 = RTCP_SSRC;
    net_send32(fd, tmp_32);

    /* RTCP: SDES: Type CNAME = 1 */
    tmp_8 = 0x1;
    send(fd, &tmp_8, 1, 0);

    /* RTCP: SDES: Length */
    tmp_8 = 0x6;
    send(fd, &tmp_8, 1, 0);

    /* RTCP: SDES: Text (name string) */
    send(fd, "monkey", 6, 0);

    /* RTCP: SDES: END */
    tmp_8 = 0x0;
    send(fd, &tmp_8, 1, 0);

    return 0;
}

void *_rtcp_worker(void *arg)
{
    int fd = (int) arg;

    return;
    //sleep(1);
    while (1) {
        rtsp_rtcp_reports(fd);
        sleep(5);
    }
}

int rtcp_worker(int fd)
{
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&tid, &attr, _rtcp_worker,
                       (void *) fd) != 0) {
        printf("Could not launch RTCP thread\n");
        exit(1);
    }

    return 0;
}
