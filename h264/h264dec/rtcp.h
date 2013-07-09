
#include <stdint.h>
#include <sys/time.h>

#ifndef RTCP_H
#define RTCP_H

/* Payload types */
#define RTCP_SR     200   /*  sender report        */
#define RTCP_RR     201   /*  receiver report      */
#define RTCP_SDES   202   /*  source description   */
#define RTCP_BYE    203   /*  good bye             */
#define RTCP_APP    204   /*  application defined  */

/* Identification */
#define RTCP_SSRC   0x0c143e07

struct rtcp_pkg {
  /* packet header */
  uint8_t  version;
  uint8_t  padding;
  uint8_t  extension;
  uint8_t  ccrc;
  uint8_t  type;
  uint16_t length;

  /* server report */
  uint32_t ssrc;
  uint32_t ts_msw;
  uint32_t ts_lsw;
  uint32_t ts_rtp;
  uint32_t sd_pk_c;
  uint32_t sd_oc_c;

  /* source definition */
  uint32_t identifier;
  uint8_t  sdes_type;
  uint8_t  sdes_length;
  uint16_t sdes_text;
  uint8_t  sdes_type2;

  /* internal / informational */
};

int debug_rtcp;

uint32_t rtcp_dlsr();
struct rtcp_pkg *rtcp_decode(unsigned char *payload,
                             unsigned long len, int *count);
int rtcp_receiver_report(int fd);
int rtcp_receiver_report_zero(int fd);
int rtcp_receiver_desc(int fd);
int rtcp_worker(int fd);

#endif
