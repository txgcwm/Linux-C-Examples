#include "rtsp.h"
#include "global.h"
#include "md5.h"
#include "type.h"

#define RTP_DEFAULT_PORT 5004
S32 start_port = RTP_DEFAULT_PORT;


static U32 md_32(CHAR *string, S32 length)
{
	MD5_CTX context;
  	union {
	  	CHAR c[16];
	  	U32 x[4];
  	} digest;
  	U32 r;
  	S32 i;
  
	MD5Init(&context);
	MD5Update(&context, string, length);
	MD5Final((U8 *)&digest, &context);
	r=0;
	for (i=0; i<3; i++)
		r ^= digest.x[i];

	return r;
}
 
/******************************************************************************/
/*
*  According	md5  get  only random Numbers 
* Arguments:
*/
/******************************************************************************/
U32 random32(S32 type)
{
  	struct {
	  	S32 type;
	  	struct timeval tv;
	  	clock_t cpu;
	  	pid_t pid;
	  	U32 hid;
	  	uid_t uid;
	  	gid_t gid;
	  	struct utsname name;
  	} s;

  	gettimeofday(&s.tv, NULL);
  	uname(&s.name);
  	s.type=type;
  	s.cpu=clock();
  	s.pid=getpid();
  	s.hid=gethostid();
  	s.uid=getuid();
  	s.gid=getgid();

  	return md_32((CHAR *)&s, sizeof(s));
}

  /******************************************************************************/
 /*
  *  According  client_port  get  server_port  (rtp/rtcp)
  * Arguments:
  *     cur_conn_num :    current connect number
  */
 /******************************************************************************/
 S32 get_server_port(S32 cur_conn_num)
{
	rtsp[cur_conn_num]->cmd_port.rtp_ser_port=(rtsp[cur_conn_num]->cmd_port.rtp_cli_port-start_port)/2+start_port;
	rtsp[cur_conn_num]->cmd_port.rtcp_ser_port=rtsp[cur_conn_num]->cmd_port.rtp_ser_port+1;

	return 0;
}

S32 get_sessionid(S32 cur_conn_num)
{
#ifdef WIN32
	rtsp[cur_conn_num]->session_id = rand();
#else
	rtsp[cur_conn_num]->session_id = 1 + (S32) (10.0 * rand() / (100000 + 1.0));
#endif

	if (rtsp[cur_conn_num]->session_id == 0) {
		rtsp[cur_conn_num]->session_id++;
	}
}

 /******************************************************************************/
/*
 *	set describe command  response buffer
 * Arguments:
 *     status   rtsp status
 *	   cur_conn_num :	 current connect number
 */
/******************************************************************************/
S32 send_setup_reply(S32 status, S32 cur_conn_num){
	CHAR temp[30];
	CHAR ttl[4];

	if(!rtsp[cur_conn_num]->out_buffer)
		return -1;

	/* build a reply message */
	sprintf(rtsp[cur_conn_num]->out_buffer, "%s %d %s"RTSP_EL"CSeq: %d"RTSP_EL"Server: %s/%s"RTSP_EL, RTSP_VER, status, (CHAR *)get_stat(status), rtsp[cur_conn_num]->rtsp_cseq, PACKAGE,
		VERSION);
	add_time_stamp(rtsp[cur_conn_num]->out_buffer, 0);
	strcat(rtsp[cur_conn_num]->out_buffer, "Session: ");
	sprintf(temp, "%d", rtsp[cur_conn_num]->session_id);
	strcat(rtsp[cur_conn_num]->out_buffer, temp);
	strcat(rtsp[cur_conn_num]->out_buffer, RTSP_EL);
	/**** unicast  ****/
	strcat(rtsp[cur_conn_num]->out_buffer, "Transport: RTP/AVP;unicast;client_port=");
	sprintf(temp, "%d", rtsp[cur_conn_num]->cmd_port.rtp_cli_port);
	strcat(rtsp[cur_conn_num]->out_buffer, temp);
	strcat(rtsp[cur_conn_num]->out_buffer, "-");
	sprintf(temp, "%d", rtsp[cur_conn_num]->cmd_port.rtcp_cli_port);
	strcat(rtsp[cur_conn_num]->out_buffer, temp);
	//sprintf(temp, ";source=%s", get_address());
	strcat(rtsp[cur_conn_num]->out_buffer, rtsp[0]->host_name);
	strcat(rtsp[cur_conn_num]->out_buffer, temp);
	strcat(rtsp[cur_conn_num]->out_buffer, ";server_port=");
	sprintf(temp, "%d", rtsp[cur_conn_num]->cmd_port.rtp_ser_port);
	strcat(rtsp[cur_conn_num]->out_buffer, temp);
	strcat(rtsp[cur_conn_num]->out_buffer, "-");
	sprintf(temp, "%d", rtsp[cur_conn_num]->cmd_port.rtcp_ser_port);
	strcat(rtsp[cur_conn_num]->out_buffer, temp);
	sprintf(temp, ";ssrc=%u", rtsp[cur_conn_num]->cmd_port.ssrc);/*xxx*/
	strcat(rtsp[cur_conn_num]->out_buffer, temp);	
	strcat(rtsp[cur_conn_num]->out_buffer,";ttl=");
	sprintf(ttl,"%d",(S32)DEFAULT_TTL);
	strcat(rtsp[cur_conn_num]->out_buffer,ttl);	
	strcat(rtsp[cur_conn_num]->out_buffer, RTSP_EL);
	strcat(rtsp[cur_conn_num]->out_buffer, RTSP_EL);

	if(write(rtsp[cur_conn_num]->cli_rtsp.cli_fd, rtsp[cur_conn_num]->out_buffer, strlen(rtsp[cur_conn_num]->out_buffer))<0){
		printf("send_setup_reply error\n");
		return -1;
	}

	return 0;
}

  /******************************************************************************/
 /*
  *  setup command processing functions 
  * Arguments:
  *     cur_conn_num :    current connect number
  */
 /******************************************************************************/
S32 rtsp_setup(S32 cur_conn_num)
{
	CHAR *p = NULL;
	CHAR trash[255], line[255];

	if(!rtsp[cur_conn_num]->in_buffer)
		return -1;

	if(check_rtsp_url(cur_conn_num)<0)
		return -1;

	if(check_rtsp_filename(cur_conn_num)<0)
		return -1;

	if(get_rtsp_cseg(cur_conn_num)<0)
		return -1;
	
	if ((p = strstr(rtsp[cur_conn_num]->in_buffer, "client_port")) == NULL && strstr(rtsp[cur_conn_num]->in_buffer, "multicast") == NULL) {
		send_reply(406,cur_conn_num); /* Not Acceptable */	
		return -1;
	}

	/**** Start parsing the Transport header ****/
	if ((p = strstr(rtsp[cur_conn_num]->in_buffer, HDR_TRANSPORT)) == NULL) {
		send_reply(406,cur_conn_num);	/* Not Acceptable */
		return -1;
	}

	if (sscanf(p, "%10s%255s", trash, line) != 2) {
		send_reply(400,cur_conn_num);	/* Bad Request */
		return -1;
	}

	/****  get client rtp and rtcp port  ****/
	if(strstr(line, "client_port") != NULL) {
		p = strstr(line, "client_port");
		p = strstr(p, "=");
		sscanf(p + 1, "%d", &(rtsp[cur_conn_num]->cmd_port.rtp_cli_port));
		p = strstr(p, "-");
		sscanf(p + 1, "%d", &(rtsp[cur_conn_num]->cmd_port.rtcp_cli_port));
	}

	get_server_port(cur_conn_num);
	rtsp[cur_conn_num]->cmd_port.seq=get_randdom_seq();
	rtsp[cur_conn_num]->cmd_port.ssrc = random32(0);
	rtsp[cur_conn_num]->cmd_port.timestamp=random32(0);
	if(send_setup_reply(200, cur_conn_num) != -1) {
		return 1;
	}

	return 0;
}

