#include "rtsp.h"
#include "type.h"

  /******************************************************************************/
 /*
  *  parse	URL  
  * Arguments:
  * 	url  
  * 	server: 	   server ip addres
  *   port: 		server port
  *   file_name:  visit filename
  */
 /******************************************************************************/

 S32 is_supported_mediatype(CHAR *p,S32 cur_conn_num)
 /* Add supported format extensions here*/
 {
	 if (strcasecmp(p,".264")==0) {	 
	 	rtsp[cur_conn_num]->vist_type=0;
		return 1;
	 }	 
	 if (strcasecmp(p,".H264")==0) {	 
		rtsp[cur_conn_num]->vist_type=0;
		return 1;
	 }	
	 if (strstr(p,"h264stream")) {
	 	 rtsp[cur_conn_num]->vist_type=2;
		 return 1;
	 }
	 if (strcasecmp(p,".ps")==0){
		rtsp[cur_conn_num]->vist_type=1;
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
		if (token)
			strcpy(file_name, token);
		else
			file_name[0] = '\0';

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
 if (crlf)
	 strcat(b, "\r\n");  /* add a message header terminator (CRLF) */
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
	
	if(!rtsp[cur_conn_num]->out_buffer)
		return -1;
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
	
	if(!sdp_buff)
		return -1;
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

	if(!rtsp[cur_conn_num]->in_buffer)
		return -1;

	if(check_rtsp_url(cur_conn_num)<0)
		return -1;

	if(check_rtsp_filename(cur_conn_num)<0)
		return -1;
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
	if(get_rtsp_cseg(cur_conn_num)<0)
		return -1;

	if(get_describe_sdp(rtsp[cur_conn_num]->sdp_buffer,cur_conn_num)<0){
		return -1;
	}
	if(send_describe_reply(200,cur_conn_num)!=-1)
		return 1;


	return 0;
}


