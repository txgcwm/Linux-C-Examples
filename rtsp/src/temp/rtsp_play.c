#include "rtsp.h"
#include "type.h"

  
 /******************************************************************************/
 /*
  *  set play command  response buffer
  * Arguments:
  * 	status	 rtsp status
  *	   cur_conn_num :	 current connect number
  */
 /******************************************************************************/
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

