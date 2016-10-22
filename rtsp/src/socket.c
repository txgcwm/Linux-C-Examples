#include "rtsp.h"
#include "type.h"

 

  /******************************************************************************/
 /*
  *  init rtsp struct memory
  * Arguments:
  */
 /******************************************************************************/

S32 init_memory()
{
	S32 i;

	for(i = 0; i < MAX_CONN; i++) {
		rtsp[i] = calloc(1,sizeof(struct rtsp_buffer));
  		if(!rtsp[i]){
			return -1;	
  		}
		sem_init(&rtspd_lock[i], 0, 0);
	}

	return 0;	
}

/******************************************************************************/
/*
*  free rtsp struct memory
* Arguments:
*/
/******************************************************************************/

S32 free_memory()
{
	S32 i;
	
	for(i=0;i<MAX_CONN;i++){
 		if(rtsp[i]!=NULL){
 			free(rtsp[i]);
			rtsp[i]=NULL;
 		}
		
		sem_destroy(&rtspd_lock[i]);
	}

	return 0;	 
}

/******************************************************************************/
/*
 *	close  rtsp  socket fd 
 * Arguments:
 */
/******************************************************************************/
S32 close_rtsp_fd()
{
	close(rtsp[0]->fd.rtspfd);
}

/******************************************************************************/
/*
 * Receiving thread, for rtsp client  command
 * Arguments: NULL
 */
/******************************************************************************/
void *vd_rtsp_procin(void *arg)
{
	pthread_detach(pthread_self());

	while(1){
		printf("server waiting \n");
		/*  Accept a connection.	*/
		tcp_accept();
	}

	pthread_exit(NULL);
}

/******************************************************************************/
/*
 *	create rtsp socket 
 * Arguments:
 *      host    - name of host to which connection is desired
 *      port - port associated with the desired port
 *      type - SOCK_STREAM or SOCK_DGRAM
 */
/******************************************************************************/
S32 create_sercmd_socket(const CHAR *host, const CHAR *port,S32 type)
{
    /*  Create a socket for the server.  */
    S32 server_len;
    S32 reuse=1;
    struct sockaddr_in server_address;

	if(!host)
	    return -1;

	rtsp[0]->fd.rtspfd = socket(AF_INET, type, 0);
	if(rtsp[0]->fd.rtspfd<0){
    	printf("socket() error in create_sercmd_socket.\n" );
	}
    /**** set address reuse ****/    
  	(void) setsockopt(rtsp[0]->fd.rtspfd, SOL_SOCKET, SO_REUSEADDR, &reuse,sizeof(reuse));
  	server_address.sin_family = AF_INET;
  	server_address.sin_addr.s_addr = inet_addr(host);
  	server_address.sin_port = htons(atoi(port));
  	server_len = sizeof(server_address); 
  	bind(rtsp[0]->fd.rtspfd, (struct sockaddr *)&server_address, server_len);
  	listen(rtsp[0]->fd.rtspfd, 5);
  	printf("rtsp server listen sucess\n");
  	/***** rtsp cmd thread******/
  	if (pthread_create(&rtsp[0]->pth.rtsp_vthread1, NULL, vd_rtsp_procin,NULL) < 0){			  
		printf("pthread_create vd_rtsp_procin thread error\n");
	  	return -1;
  	}
	return 0;
}

/******************************************************************************/
/*
 * RTSP command match
 *     method:    rtsp command
 *     cur_conn_num:  current  connection status
 */
/******************************************************************************/
 S32 rtsp_cmd_match(S32 method, S32 cur_conn_num) 
{
	switch(method){
		case 1:
			if(rtsp_options(cur_conn_num) <= 0) {
				printf("options command response not sucessfull\n");
				close(rtsp[cur_conn_num]->cli_rtsp.cli_fd);
				rtsp[cur_conn_num]->rtspd_status=0x01;
				rtsp[cur_conn_num]->is_runing=0;
				sem_post(&rtspd_lock[cur_conn_num]);
				return -1;
			} else {
				printf("options command response  sucessfull\n");
				rtsp[cur_conn_num]->rtspd_status=0x02;
			}
			break;
	 
		case 2:
			if(rtsp_describe(cur_conn_num)<=0){
				printf("describe command response not sucessfull\n");
				close(rtsp[cur_conn_num]->cli_rtsp.cli_fd);
				rtsp[cur_conn_num]->rtspd_status=0x03;
				rtsp[cur_conn_num]->is_runing=0;
				sem_post(&rtspd_lock[cur_conn_num]);
				return -1;
			} else{
				printf("describe command response	sucessfull\n");
				rtsp[cur_conn_num]->rtspd_status=0x04;
			}
			break;
	
		case 3:
			if(rtsp_setup(cur_conn_num)<=0){
				printf("setup command response not sucessfull\n");
				close(rtsp[cur_conn_num]->cli_rtsp.cli_fd);
				rtsp[cur_conn_num]->rtspd_status=0x05;
				rtsp[cur_conn_num]->is_runing=0;
				sem_post(&rtspd_lock[cur_conn_num]);
				return -1;
			} else {
				printf("setup command response  sucessfull\n");
				rtsp[cur_conn_num]->rtspd_status=0x06;
			}
			break;
			
		case 4:
			if(rtsp_play(cur_conn_num)<=0){
				printf("play command response not sucessfull\n");
				close(rtsp[cur_conn_num]->cli_rtsp.cli_fd);
				rtsp[cur_conn_num]->rtspd_status=0x07;
				rtsp[cur_conn_num]->is_runing=0;
				sem_post(&rtspd_lock[cur_conn_num]);
				return -1;
			} else {
				printf("play command response	 sucessfull\n");
				/****	deal with  RTP RTCP agreement function ****/
				rtsp[cur_conn_num]->is_runing=1;
				rtsp[cur_conn_num]->rtspd_status=0x08;
				sem_post(&rtspd_lock[cur_conn_num]);
			}
			break;
		case 5:
			if(rtsp_terardown(cur_conn_num)<=0){
				printf("terardown command response not sucessfull\n");
				close(rtsp[cur_conn_num]->cli_rtsp.cli_fd);
				rtsp[cur_conn_num]->rtspd_status=0x11;
				rtsp[cur_conn_num]->is_runing=0;
				return -1;
			} else{
				printf("terardown command response	sucessfull\n");
				rtsp[cur_conn_num]->is_runing=0;
				close(rtsp[cur_conn_num]->cli_rtsp.cli_fd);
				rtsp[cur_conn_num]->rtspd_status=0x12;
				return -1;
			}
			break;

		default:
			printf("not match rtsp command\n");
			break;
	}


	return 0;
}

 /******************************************************************************/
 /*
  * Receiving thread, for rtsp client  command
  * Arguments: NULL
  */
 /******************************************************************************/
 void *vd_rtsp_proc(void *arg)
{
	S32 method,free_conn=0;
	
	pthread_detach(pthread_self());

	free_conn=(S32)arg;

	while(1){
	 	if(tcp_read(rtsp[free_conn]->cli_rtsp.cli_fd, 
			rtsp[free_conn]->in_buffer, sizeof(rtsp[free_conn]->in_buffer))<0) {
			break;
		}

		method=get_rtsp_method(free_conn);
		if(rtsp_cmd_match(method,free_conn)<0) {
			sem_wait(&rtspd_semop);
			set_free_conn_status(free_conn,0);
			sem_post(&rtspd_semop);
			break;
		} else {
			sem_wait(&rtspd_semop);
			set_free_conn_status(free_conn,1);
			sem_post(&rtspd_semop);
	    }
	}

QUIT:
	pthread_exit(NULL);
}

/******************************************************************************/
/*
 *	Set free connection status
 * Arguments:
 *     cur_conn :   current  connection 
 *     cur_status:  current  connection status
 */
/******************************************************************************/
S32 set_free_conn_status(S32 cur_conn, S32 cur_status)
{
	S32 i,j=0;

	for(i=0;i<MAX_CONN;i++){
		if((i==cur_conn)){
			rtsp[i]->conn_status=cur_status;	
		}
	}

	return 0;
}

/******************************************************************************/
/*
 *	get free connection status
 * Arguments:
 *     NULL
 */
/******************************************************************************/
S32 get_free_conn_status()
{
	S32 i,j=0;

	for(i=0;i<MAX_CONN;i++){
		if(rtsp[i]->conn_status==0){
			return i;	
		}
	}
	
	return -1;	
}

/******************************************************************************/
/*
 *	create tcp_accept 
 * Arguments:
 *     NULL
 */
/******************************************************************************/
S32 tcp_accept()
{
	S32 client_len,free_conn=0;
    struct sockaddr_in client_address;

	free_conn=get_free_conn_status();
	if(free_conn==-1){
		printf("waring: maximum number of connections\n");
		return -1;
	}
	rtsp[free_conn]->cli_rtsp.conn_num=free_conn;

  	client_len = sizeof(client_address);
  	rtsp[free_conn]->cli_rtsp.cli_fd= accept(rtsp[0]->fd.rtspfd, (struct sockaddr *)&client_address, &client_len);

	sem_wait(&rtspd_semop);
	strcpy(rtsp[free_conn]->cli_rtsp.cli_host,inet_ntoa(client_address.sin_addr));
	sem_post(&rtspd_semop);
	//sem_post(&rtspd_accept_lock);
	pthread_mutex_lock(&rtspd_mutex);
	pthread_cond_signal(&rtspd_cond);
	pthread_mutex_unlock(&rtspd_mutex);

	return 0;
}

S32 tcp_read(S32 fd, void *buf, S32 length)
{
    S32 bytes_read;
	S32 bytes_left;
	CHAR *ptr = buf;
	bytes_left = length;

	while(1) {
	    bytes_read = read(fd,ptr,bytes_left);
		if(bytes_read<0) {
		    if(EINTR==errno) {
				continue;
		    } else {
				return (-1);
		    }
		} else {
			break;
		}
	}

	return bytes_read;
}

S32 tcp_write(S32 fd, void *buf, S32 length)
{
    S32 bytes_write;
	S32 bytes_left = length;
	CHAR *ptr = buf;

	while(bytes_left>0) {
	    bytes_write = write(fd,ptr,bytes_left);
		if(bytes_write<0) {
		    if(EINTR==errno) {
				continue;
		    } else {
				return (-1);
			}
		}

		bytes_left -= bytes_write;
		ptr += bytes_write;
	}

	return (length - bytes_left);
}

