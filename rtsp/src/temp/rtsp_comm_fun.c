#include "rtsp.h"
#include "type.h"

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
		send_reply(400,cur_conn_num);
		return -1;
	} 
	else {
		if(sscanf(p, "%254s %d", trash, &(rtsp[cur_conn_num]->rtsp_cseq))!=2){
			/**** not find CSeq value send 400 error ****/	
			send_reply(400,cur_conn_num);
			return -1;
		}
	}
	#if 0
	if( strstr(rtsp[cur_conn_num]->in_buffer, "ClientChallenge")){
		return -1;
	}
	#endif
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



