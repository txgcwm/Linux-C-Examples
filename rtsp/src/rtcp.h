#ifndef __RTCP_H
#define __RTCP_H

#include "type.h"


#if 0
typedef enum {
	SR=200,
	RR=201,
	SDES=202,
	BYE=203,
	APP=204
} rtcp_pkt_type;

typedef enum {
	CNAME=1,
	NAME=2,
	EMAIL=3,
	PHONE=4,
	LOC=5,
	TOOL=6,
	NOTE=7,
	PRIV=8		
} rtcp_info;


struct rtcp_common_t{
#ifdef WORDS_BIGENDIAN
		U32 ver:2;
		U32 pad:1;
		U32 count:5;
#else
		
		U32 count:5;
		U32 pad:1;
		U32 ver:2;
#endif


  U32 pt:8;
  U32 len:16;
};

 struct rtcp_rr_t{
  U32 ssrc;
  U32 fraction:8;
  S32 lost:24;
  U32 last_seq;
  U32 jitter;
  U32 last_sr;
  U32 dlsr;
} rr_t;

struct rr{
  U32 ssrc;
  struct rtcp_rr_t rr[1];
} ;

 struct rtcp_pkt{
   struct rtcp_common_t comm;
   struct rr revc_port;
} rtcp_pkt;

S32 bulid_rtcp_sr(struct rtcp_pkt *pkt);
#endif
typedef enum {
	SR=200,
	RR=201,
	SDES=202,
	BYE=203,
	APP=204
} rtcp_pkt_type;

typedef enum {
	CNAME=1,
	NAME=2,
	EMAIL=3,
	PHONE=4,
	LOC=5,
	TOOL=6,
	NOTE=7,
	PRIV=8		
} rtcp_info;
	
typedef struct _RTCP_header {
#ifdef WORDS_BIGENDIAN
	U32 version:2;
	U32 padding:1;
	U32 count:5;		//SC oppure RC

#else
	U32 count:5;		//SC oppure RC
	U32 padding:1; 		
	U32 version:2;
#endif
	U32 pt:8;
	U32 length:16;
} RTCP_header;

typedef struct _RTCP_header_SR {
	U32 ssrc;		
	U32 ntp_timestampH;
	U32 ntp_timestampL;
	U32 rtp_timestamp;
	U32 pkt_count; 		
	U32 octet_count;
} RTCP_header_SR;

typedef struct _RTCP_header_RR {
	U32 ssrc;
} RTCP_header_RR;	

typedef struct _RTCP_header_SR_report_block {
	UL64 ssrc;
	U8 fract_lost;		
	U8 pck_lost[3];
	U32 h_seq_no;
	U32 jitter;
	U32 last_SR;
	U32 delay_last_SR; 		
} RTCP_header_SR_report_block;	
		
typedef struct _RTCP_header_SDES {
	U32 ssrc;
	U8 attr_name;
	U8 len;	
	CHAR name[];
} RTCP_header_SDES;

typedef struct _RTCP_header_BYE {
	U32 ssrc;
	U8 length;
} RTCP_header_BYE;

struct rtcp_pkt{
   RTCP_header comm;
   RTCP_header_SDES sdec;
} rtcp_pkt;



#endif

