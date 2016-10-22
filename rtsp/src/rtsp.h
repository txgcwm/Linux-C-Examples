#ifndef _RTSP_RTSP_H
#define _RTSP_RTSP_H

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>

#include "type.h"



#define RTSP_EL "\r\n"
#define RTSP_VER "RTSP/1.0"
#define HDR_REQUIRE "Require"
#define HDR_ACCEPT "Accept"
#define PACKAGE "rtspd"
#define VERSION "1.0"
#define SDP_EL "\r\n"
#define HDR_TRANSPORT "Transport"
#define HDR_SESSION "Session"


#define RTSP_BUFFERSIZE 4096
#define MAX_DESCR_LENGTH 4096
#define DEFAULT_TTL 32
#define HDR_CSEQ "CSeq"
#define MAX_CONN 100
extern sem_t rtspd_semop;
extern sem_t rtspd_lock[MAX_CONN];
extern sem_t rtspd_accept_lock;
extern pthread_cond_t rtspd_cond;
extern pthread_mutex_t rtspd_mutex;

extern struct rtsp_buffer *rtsp[MAX_CONN];
struct rtsp_port{
	S32 rtp_cli_port;
	S32 rtcp_cli_port;
	S32 rtp_ser_port;
	S32 rtcp_ser_port;
	U32 ssrc;
	U32 timestamp;
	U32 frame_rate_step;
	U16 seq;
};

struct rtsp_fd{
	S32 rtspfd;
	S32 video_rtp_fd;
	S32 video_rtcp_fd;
	S32 audio_rtp_fd;
	S32 audio_rtcp_fd;
};

struct rtsp_th{
	pthread_t rtsp_vthread;	
	pthread_t rtsp_vthread1;
	pthread_t rtp_vthread;
	pthread_t rtcp_vthread;
};

struct rtsp_cli{
	S32 cli_fd;
	S32 conn_num;
	CHAR cli_host[128];
};

struct rtsp_buffer {
	S32 payload_type; /* 96 h263/h264*/
	S32 session_id;
	U32 rtsp_deport;
	U32 rtsp_um_stat;  /**** 0 is Unicast   1 is multicast  ****/
	U32 rtsp_cseq;
	U32 is_runing;
	U32 cur_conn;
	U32 conn_status;	
	U32 rtspd_status;
	U32 vist_type;  /****0: H264 file vist  1: PS file vist  2: h264 stream vist ****/

	struct rtsp_port cmd_port;
	struct rtsp_fd fd;
	struct rtsp_th pth;
	struct rtsp_cli cli_rtsp;
	// Buffers		
	CHAR file_name[128];
	CHAR host_name[128];
	U8 nalu_buffer[1448];
	CHAR in_buffer[RTSP_BUFFERSIZE];
	CHAR out_buffer[RTSP_BUFFERSIZE];	
	CHAR sdp_buffer[MAX_DESCR_LENGTH];
};

S32 build_rtp_nalu(U8 *inbuffer, S32 frame_size, S32 cur_conn_num);


#endif
