#include "rtsp.h"
#include "type.h"
#include "global.h"
#include "md5.h"


struct rtsp_buffer *rtsp[MAX_CONN];
 /******************************************************************************/
/*
 *	get rtsp  cseg functions 
 * Arguments:
 *     cur_conn_num :    current connect number
 */
/******************************************************************************/
S32 get_rtsp_cseg(S32 cur_conn_num)
{
	CHAR *p;
	CHAR trash[255];
	
	/****check  CSeq****/ 
	if ((p = strstr(rtsp[cur_conn_num]->in_buffer, "CSeq")) == NULL) {
		/**** not find CSeq send 400 error ****/
		send_reply(400, cur_conn_num);
		return -1;
	} else {
		if(sscanf(p, "%254s %d", trash, &(rtsp[cur_conn_num]->rtsp_cseq))!=2){
			/**** not find CSeq value send 400 error ****/	
			send_reply(400, cur_conn_num);
			return -1;
		}
	}

	return 0;
}

 /******************************************************************************/
/*
 *	check  rtsp  url functions 
 * Arguments:
 *	   cur_conn_num :	 current connect number
 */
/******************************************************************************/
S32 check_rtsp_url(S32 cur_conn_num)
{
	U16 port;
	CHAR url[128];
	CHAR object[128], server[128];
	 
	if (!sscanf(rtsp[cur_conn_num]->in_buffer, " %*s %254s ", url)) {
		send_reply(400,cur_conn_num);	/* bad request */
		return -1;
	}

	/* Validate the URL */
	if (!parse_url(url, server, &port, object)) {
		send_reply(400,cur_conn_num);	/* bad request */
		return -1;
	}
	strcpy(rtsp[0]->host_name,server);
	/****  get media file name   ****/
	if(strstr(object,"trackID")){
		strcpy(object,rtsp[0]->file_name);
	} else {
		if(strcmp(object,"")==0){
			strcpy(object,rtsp[0]->file_name);
		}
		strcpy(rtsp[0]->file_name,object);
	}

	return 0;
}

 /******************************************************************************/
/*
 *	check  rtsp  filename functions 
 * Arguments:
 *	   cur_conn_num :	 current connect number
 */
/******************************************************************************/
S32 check_rtsp_filename(S32 cur_conn_num)
{
	CHAR *p;
	S32 valid_url;
	
	if (strstr(rtsp[0]->file_name, "../")) {
		/* disallow relative paths outside of current directory. */
		send_reply(403,cur_conn_num);	 /* Forbidden */
		return -1;
	}

	if (strstr(rtsp[0]->file_name, "./")) {
		/* Disallow ./ */
		send_reply(403,cur_conn_num);	 /* Forbidden */
		return -1;
	}
	
	p = strrchr(rtsp[0]->file_name, '.');
	valid_url = 0;

	if (p == NULL) {
		send_reply(415,cur_conn_num);	 /* Unsupported media type */
		return -1;
	} else {
		valid_url = is_supported_mediatype(p,cur_conn_num);
	}

	if (!valid_url) {
		send_reply(415,cur_conn_num);	 /* Unsupported media type */
		return -1;
	}

	return 0;
}

CHAR *get_stat(S32 err)
{
	struct {
		CHAR *token;
		S32 code;
	} status[] = {
		{
		"Continue", 100}, {
		"OK", 200}, {
		"Created", 201}, {
		"Accepted", 202}, {
		"Non-Authoritative Information", 203}, {
		"No Content", 204}, {
		"Reset Content", 205}, {
		"Partial Content", 206}, {
		"Multiple Choices", 300}, {
		"Moved Permanently", 301}, {
		"Moved Temporarily", 302}, {
		"Bad Request", 400}, {
		"Unauthorized", 401}, {
		"Payment Required", 402}, {
		"Forbidden", 403}, {
		"Not Found", 404}, {
		"Method Not Allowed", 405}, {
		"Not Acceptable", 406}, {
		"Proxy Authentication Required", 407}, {
		"Request Time-out", 408}, {
		"Conflict", 409}, {
		"Gone", 410}, {
		"Length Required", 411}, {
		"Precondition Failed", 412}, {
		"Request Entity Too Large", 413}, {
		"Request-URI Too Large", 414}, {
		"Unsupported Media Type", 415}, {
		"Bad Extension", 420}, {
		"Invalid Parameter", 450}, {
		"Parameter Not Understood", 451}, {
		"Conference Not Found", 452}, {
		"Not Enough Bandwidth", 453}, {
		"Session Not Found", 454}, {
		"Method Not Valid In This State", 455}, {
		"Header Field Not Valid for Resource", 456}, {
		"Invalid Range", 457}, {
		"Parameter Is Read-Only", 458}, {
		"Unsupported transport", 461}, {
		"Internal Server Error", 500}, {
		"Not Implemented", 501}, {
		"Bad Gateway", 502}, {
		"Service Unavailable", 503}, {
		"Gateway Time-out", 504}, {
		"RTSP Version Not Supported", 505}, {
		"Option not supported", 551}, {
		"Extended Error:", 911}, {
		NULL, -1}
	};
	
	S32 i;
	for (i = 0; status[i].code != err && status[i].code != -1; ++i);

	return status[i].token;
}

 /******************************************************************************/
/*
 *	send rtsp error response mseeage 
 * Arguments:
 *     errornumber   rtsp command  err number
 *     cur_conn_num :    current connect number
 */
/******************************************************************************/

S32 send_reply(S32 errornumber,S32 cur_conn_num)
{
	 CHAR error_buff[1024]="";
	 
	 sprintf(error_buff, "%s %d %s"RTSP_EL"CSeq: %d"RTSP_EL, RTSP_VER, errornumber, (CHAR *)get_stat(errornumber), rtsp[cur_conn_num]->rtsp_cseq);
	 //---patch coerenza con rfc in caso di errore
	 // strcat(b, "\r\n");
	 strcat(error_buff, RTSP_EL);
	 if(write(rtsp[cur_conn_num]->cli_rtsp.cli_fd, error_buff, strlen(error_buff))<0) {
	 	printf("send rtspd reply packet error\n");
		return -1;
 	}

	return 0;
}

 /******************************************************************************/
/*
 *	obtain RTSP command type  
 * Arguments:
 *     inbuf     receive rtsp client buff
 *     cur_conn_num :    current connect number
 */
/******************************************************************************/
S32 get_rtsp_method(S32 cur_conn_num)
{
	CHAR method[32];
	
	if(!rtsp[cur_conn_num]->in_buffer) {
		return -1;
	}

	sscanf(rtsp[cur_conn_num]->in_buffer, "%31s", method);	
	if (strcmp(method, "OPTIONS") == 0) {
		return 1;
	}

	if (strcmp(method, "DESCRIBE") == 0) {
		return 2;
	}

	if (strcmp(method, "SETUP") == 0) {
		return 3;
	}

	if (strcmp(method, "PLAY") == 0) {
		return 4;
	}

	if (strcmp(method, "TEARDOWN") == 0) {
		return 5;
	}
	
	return 0;	
}

S32 is_supported_mediatype(CHAR *p,S32 cur_conn_num)
 /* Add supported format extensions here*/
 {
	if (strcasecmp(p,".264") == 0) {	 
	 	rtsp[cur_conn_num]->vist_type = 0;
		return 1;
	}	 
	
	if (strcasecmp(p,".H264") == 0) {
		rtsp[cur_conn_num]->vist_type = 0;
		return 1;
	}	
	
	if (strstr(p,"h264stream")) {
	 	rtsp[cur_conn_num]->vist_type = 2;
		return 1;
	}
	
	if (strcasecmp(p,".ps") == 0){
		rtsp[cur_conn_num]->vist_type = 1;
		return 1;
	}

	return 0;
 }

 /******************************************************************************/
/*
 *	parse  URL	
 * Arguments:
 *	   url	
 *	   server:		  server ip addres
 *	 port:		   server port
 *	 file_name:  visit filename
 */
/******************************************************************************/

/* return 1 if the URL is valid, 0 otherwise*/
S32 parse_url(const CHAR *url, CHAR *server, U16 *port, CHAR *file_name)
// Note: this routine comes from OMS
{
	/* expects format '[rtsp://server[:port/]]filename' */

	S32 valid_url = 0;
	CHAR *token,*port_str;
	CHAR temp_url[128]="";
	
	/* copy url */
	strcpy(temp_url, url);
	if (strncmp(temp_url, "rtsp://", 7) == 0) {
	   	token = strtok(&temp_url[7], " :/\t\n");
	   	strcpy(server, token);
	   	port_str = strtok(&temp_url[strlen(server) + 7 + 1], " /\t\n");
	   	if (port_str)
			*port = (U16) atol(port_str);
		else
			*port = 554;
		valid_url = 1;

		token = strtok(NULL, " ");
		if (token) {
			strcpy(file_name, token);
		} else {
			file_name[0] = '\0';
		}

	} else {
		/* try just to extract a file name */
	}
	
	return valid_url;
}

CHAR *get_hostname()
{
	S32 l;
	
	gethostname(rtsp[0]->host_name,sizeof(rtsp[0]->host_name));
	l=strlen(rtsp[0]->host_name);
	if (getdomainname(rtsp[0]->host_name+l+1,sizeof(rtsp[0]->host_name)-l)!=0) {
		rtsp[0]->host_name[l]='.';
	}

	return rtsp[0]->host_name;
}

/******************************************************************************/
/*
 *	create time_stamp
 * Arguments:
 *     b:     time_stamp buffer
 *     crlf     
 */
/******************************************************************************/
VOID add_time_stamp(CHAR *b, S32 crlf)
{
 	struct tm *t;
 	time_t now;

 /*
  * concatenates a null terminated string with a
  * time stamp in the format of "Date: 23 Jan 1997 15:35:06 GMT"
  */
 	now = time(NULL);
 	t = gmtime(&now);
 	strftime(b + strlen(b), 38, "Date: %a, %d %b %Y %H:%M:%S GMT"RTSP_EL, t);
 	if (crlf) {
	 	strcat(b, "\r\n");  /* add a message header terminator (CRLF) */
 	}
}

/******************************************************************************/
/*
 *	set describe command  response buffer
 * Arguments:
 *     status   rtsp status
 *     cur_conn_num :    current connect number
 */
/******************************************************************************/
S32 send_describe_reply(S32 status,S32 cur_conn_num){
	
	if(!rtsp[cur_conn_num]->out_buffer) {
		return -1;
	}

	/*describe*/
	sprintf(rtsp[cur_conn_num]->out_buffer, "%s %d %s"RTSP_EL"CSeq: %d"RTSP_EL"Server: %s/%s"RTSP_EL, RTSP_VER, status, (CHAR *)get_stat(status), rtsp[cur_conn_num]->rtsp_cseq, PACKAGE, VERSION);	
	add_time_stamp(rtsp[cur_conn_num]->out_buffer, 0);
	strcat(rtsp[cur_conn_num]->out_buffer, "Content-Type: application/sdp"RTSP_EL);
	sprintf(rtsp[cur_conn_num]->out_buffer + strlen(rtsp[cur_conn_num]->out_buffer), "Content-Base: rtsp://%s/%s/"RTSP_EL,rtsp[0]->host_name, rtsp[0]->file_name);
	sprintf(rtsp[cur_conn_num]->out_buffer + strlen(rtsp[cur_conn_num]->out_buffer), "Content-Length: %d"RTSP_EL, strlen(rtsp[cur_conn_num]->sdp_buffer));
	strcat(rtsp[cur_conn_num]->out_buffer, RTSP_EL);
	/**** concatenate description ****/
	strcat(rtsp[cur_conn_num]->out_buffer, rtsp[cur_conn_num]->sdp_buffer);
	
	if(write(rtsp[cur_conn_num]->cli_rtsp.cli_fd, rtsp[cur_conn_num]->out_buffer, strlen(rtsp[cur_conn_num]->out_buffer))<0){
		printf("send_describe_reply error\n");
		return -1;
	}

	return 0;
}

  /******************************************************************************/
 /*
  *  get  sdp   message 
  * Arguments:
  * 	
  * 	
  */
 /******************************************************************************/
 CHAR *get_SDP_user_name(CHAR *buffer)
 {
	strcpy(buffer,PACKAGE);
	return buffer;
 }

float NTP_time(time_t t)
{
	return (float)t+2208988800U;
}

CHAR *get_SDP_session_id(CHAR *buffer)
{	 
	buffer[0]='\0';
	sprintf(buffer,"%.0f",NTP_time(time(NULL)));
	return buffer;  
}

CHAR *get_SDP_version(CHAR *buffer)
{
	buffer[0]='\0'; 
	sprintf(buffer,"%.0f",NTP_time(time(NULL)));
	return buffer;
}

CHAR *get_address()
{
   static CHAR	   Ip[256];
   CHAR server[256];
   u_char		   addr1, addr2, addr3, addr4, temp;
   u_long		   InAddr;
   struct hostent *host;

   gethostname(server,256);
   host = gethostbyname(server);
 
   temp = 0;
   InAddr = *(U32 *) host->h_addr;
   addr4 = (U8) ((InAddr & 0xFF000000) >> 0x18);
   addr3 = (U8) ((InAddr & 0x00FF0000) >> 0x10);
   addr2 = (U8) ((InAddr & 0x0000FF00) >> 0x8);
   addr1 = (U8) (InAddr & 0x000000FF);
#if (BYTE_ORDER == BIG_ENDIAN)
   temp = addr1;
   addr1 = addr4;
   addr4 = temp;
   temp = addr2;
   addr2 = addr3;
   addr3 = temp;
#endif
 
   sprintf(Ip, "%d.%d.%d.%d", addr1, addr2, addr3, addr4);

   return Ip;
}

 /******************************************************************************/
/*
 *	get  describe sdp message 
 * Arguments:
 *     cur_conn_num :    current connect number
 *     
 */
/******************************************************************************/

S32 get_describe_sdp(CHAR *sdp_buff,S32 cur_conn_num)
{
	CHAR s[30];
	
	if(!sdp_buff) {
		return -1;
	}

	strcpy(sdp_buff, "v=0"SDP_EL);	
	strcat(sdp_buff, "o=");
	strcat(sdp_buff,get_SDP_user_name(s));
	strcat(sdp_buff," ");
	strcat(sdp_buff, get_SDP_session_id(s));
	strcat(sdp_buff," ");
	strcat(sdp_buff, get_SDP_version(s));
	strcat(sdp_buff, SDP_EL);
   	strcat(sdp_buff, "c=");
   	strcat(sdp_buff, "IN ");		/* Network type: Internet. */
   	strcat(sdp_buff, "IP4 ");		/* Address type: IP4. */
	//strcat(sdp_buff, get_address());
	strcat(sdp_buff, rtsp[0]->host_name);
	strcat(sdp_buff, SDP_EL);
	strcat(sdp_buff, "s=RTSP Session"SDP_EL);
	sprintf(sdp_buff + strlen(sdp_buff), "i=%s %s Streaming Server"SDP_EL, PACKAGE, VERSION);
   	sprintf(sdp_buff + strlen(sdp_buff), "u=%s"SDP_EL, rtsp[0]->file_name);
   	strcat(sdp_buff, "t=0 0"SDP_EL);	
	/**** media specific ****/
	strcat(sdp_buff,"m=");
	strcat(sdp_buff,"video 0");
	strcat(sdp_buff," RTP/AVP "); /* Use UDP */
	rtsp[cur_conn_num]->payload_type=96;
	sprintf(sdp_buff + strlen(sdp_buff), "%d"SDP_EL, rtsp[cur_conn_num]->payload_type);
	if (rtsp[cur_conn_num]->payload_type>=96) {
		/**** Dynamically defined payload ****/
		strcat(sdp_buff,"a=rtpmap:");
		sprintf(sdp_buff + strlen(sdp_buff), "%d", rtsp[cur_conn_num]->payload_type);
		strcat(sdp_buff," ");	
		strcat(sdp_buff,"H264/90000");
		strcat(sdp_buff, SDP_EL);
		strcat(sdp_buff,"a=fmtp:96 packetization-mode=1;profile-level-id=1EE042;sprop-parameter-sets=QuAe2gLASRA=,zjCkgA==");
		strcat(sdp_buff, SDP_EL);
		strcat(sdp_buff,"a=control:");
		sprintf(sdp_buff + strlen(sdp_buff),"rtsp://%s/%s/trackID=0",rtsp[0]->host_name,rtsp[0]->file_name);
		strcat(sdp_buff, SDP_EL);

	}
	strcat(sdp_buff, SDP_EL);

	return 0;
}

  /******************************************************************************/
 /*
  *  describe command processing functions 
  * Arguments:
  *     cur_conn_num :    current connect number
  */
 /******************************************************************************/
S32 rtsp_describe(S32 cur_conn_num){

	if(!rtsp[cur_conn_num]->in_buffer) {
		return -1;
	}

	if(check_rtsp_url(cur_conn_num)<0) {
		return -1;
	}

	if(check_rtsp_filename(cur_conn_num)<0) {
		return -1;
	}

	// Disallow Header REQUIRE
	if (strstr(rtsp[cur_conn_num]->in_buffer, HDR_REQUIRE)) {
		send_reply(551,cur_conn_num);	/* Option not supported */
		return -1;
	}

	/* Get the description format. SDP is recomended */
	if (strstr(rtsp[cur_conn_num]->in_buffer, HDR_ACCEPT) != NULL) {
		if (strstr(rtsp[cur_conn_num]->in_buffer, "application/sdp") != NULL) {
			//descr_format = df_SDP_format;		
		} else {
			// Add here new description formats
			send_reply(551,cur_conn_num);	/* Option not supported */
			return -1;
		}
	}

	if(get_rtsp_cseg(cur_conn_num)<0) {
		return -1;
	}

	if(get_describe_sdp(rtsp[cur_conn_num]->sdp_buffer,cur_conn_num) < 0) {
		return -1;
	}

	if(send_describe_reply(200,cur_conn_num)!=-1) {
		return 1;
	}

	return 0;
}

S32 set_options_reply(S32 status,S32 cur_conn_num)
{
	if(!rtsp[cur_conn_num]->out_buffer) {
		return -1;
	}

	sprintf(rtsp[cur_conn_num]->out_buffer, "%s %d %s"RTSP_EL"CSeq: %d"RTSP_EL, RTSP_VER, status, (CHAR *)get_stat(status), rtsp[cur_conn_num]->rtsp_cseq);
	strcat(rtsp[cur_conn_num]->out_buffer, "Public: OPTIONS,DESCRIBE,SETUP,PLAY,PAUSE,TEARDOWN"RTSP_EL);
	strcat(rtsp[cur_conn_num]->out_buffer, RTSP_EL);
	if(write(rtsp[cur_conn_num]->cli_rtsp.cli_fd, rtsp[cur_conn_num]->out_buffer, strlen(rtsp[cur_conn_num]->out_buffer))<0){
		printf("set_options_reply error\n");
		return -1;
	}

	return 0;
}

 /******************************************************************************/
/*
 *	options command processing functions 
 * Arguments:
 *     cur_conn_num :    current connect number
 */
/******************************************************************************/
S32 rtsp_options(S32 cur_conn_num){
	
	if(!rtsp[cur_conn_num]->in_buffer) {
		return -1;
	}

	if(get_rtsp_cseg(cur_conn_num)!=-1){
		set_options_reply(200, cur_conn_num);
		return 1;
	}
		
	return 0;
}

S32 send_play_reply(S32 status,S32 cur_conn_num)
{
	CHAR temp[255];
	
	if(!rtsp[cur_conn_num]->out_buffer) {
		return -1;
	}
	
	/* build a reply message */
	sprintf(rtsp[cur_conn_num]->out_buffer, "%s %d %s"RTSP_EL"CSeq: %d"RTSP_EL"Server: %s/%s"RTSP_EL, RTSP_VER, status, (CHAR *)get_stat(status), rtsp[cur_conn_num]->rtsp_cseq, PACKAGE,
		VERSION);
	add_time_stamp(rtsp[cur_conn_num]->out_buffer, 0);
	strcat(rtsp[cur_conn_num]->out_buffer, "Session: ");
	sprintf(temp, "%d", rtsp[cur_conn_num]->session_id);
	strcat(rtsp[cur_conn_num]->out_buffer, temp);
	strcat(rtsp[cur_conn_num]->out_buffer, RTSP_EL);
	strcat(rtsp[cur_conn_num]->out_buffer, RTSP_EL);
	if(write(rtsp[cur_conn_num]->cli_rtsp.cli_fd, rtsp[cur_conn_num]->out_buffer, strlen(rtsp[cur_conn_num]->out_buffer))<0){
		printf("send_play_reply error\n");
		return -1;
	}

	return 0;
}

 /******************************************************************************/
/*
 *	set play command  response buffer
 * Arguments:
 *	   cur_conn_num :	 current connect number
 */
/******************************************************************************/
S32 rtsp_play(S32 cur_conn_num)
{
	CHAR *p = NULL;
	CHAR  trash[255];
	
	if(!rtsp[cur_conn_num]->in_buffer) {
		return -1;
	}
	/**** Parse the input message ****/

	if(check_rtsp_url(cur_conn_num) < 0){
		return -1;
	}

	if(get_rtsp_cseg(cur_conn_num) < 0){
		return -1;
	}

	if(check_rtsp_filename(cur_conn_num) < 0){
		return -1;
	}
	
	// If we get a Session hdr, then we have an aggregate control
	if ((p = strstr(rtsp[cur_conn_num]->in_buffer, HDR_SESSION)) != NULL) {
		if (sscanf(p, "%254s %d", trash, &rtsp[cur_conn_num]->session_id) != 2) {
			send_reply(454,cur_conn_num);	/* Session Not Found */
			return -1;
		}
	} else {
		send_reply(400,cur_conn_num);	/* bad request */
		return -1;
	}

	if(send_play_reply(200,cur_conn_num)!=-1)
		return 1;

	return 0;
}

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
	for (i=0; i<3; i++) {
		r ^= digest.x[i];
	}

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

	if(!rtsp[cur_conn_num]->out_buffer) {
		return -1;
	}

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

S32 send_terardown_reply(S32 status,S32 cur_conn_num)
{
	CHAR temp[255];
	
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
	strcat(rtsp[cur_conn_num]->out_buffer, RTSP_EL);
	if(write(rtsp[cur_conn_num]->cli_rtsp.cli_fd, rtsp[cur_conn_num]->out_buffer, strlen(rtsp[cur_conn_num]->out_buffer))<0){
		printf("send_terardown_reply error\n");
		return -1;
	}

	return 0;
}

 /******************************************************************************/
/*
 *	set terardown command  response buffer
 * Arguments:
 *	   cur_conn_num :	 current connect number
 */
/******************************************************************************/
S32 rtsp_terardown(S32 cur_conn_num)
{
	CHAR *p = NULL;
	CHAR  trash[255];
	if(!rtsp[cur_conn_num]->in_buffer) {
		return -1;
	}
	/**** Parse the input message ****/

	if(check_rtsp_url(cur_conn_num) < 0) {
		return -1;
	}

	if(get_rtsp_cseg(cur_conn_num) < 0) {
		return -1;
	}

	if(check_rtsp_filename(cur_conn_num) < 0) {
		return -1;
	}

	// If we get a Session hdr, then we have an aggregate control
	if ((p = strstr(rtsp[cur_conn_num]->in_buffer, HDR_SESSION)) != NULL) {
		if (sscanf(p, "%254s %d", trash, &rtsp[cur_conn_num]->session_id) != 2) {
			send_reply(454,cur_conn_num);	/* Session Not Found */
			return -1;
		}
	} else {
		send_reply(400,cur_conn_num);	/* bad request */
		return -1;
	}
	
	if(send_terardown_reply(200,cur_conn_num)!=-1) {
		return 1;
	}

	return 0;
}




