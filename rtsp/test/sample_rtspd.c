#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include "rtspd_api.h"

typedef int (*rtspd_call)(void);
pthread_t rtspd_vthread;

/* 
   signal SIGINT handler
*/
VOID sig_exit(VOID)
{
	// just S32errupt the pause()
	return;
}

int rtsp_callback()
{
	S32 free_chn,vist_type;
	while(1){
		free_chn=rtspd_freechn();
		set_framerate(25,free_chn);
		if(rtsp_proc(free_chn)<0){
			continue;
		}
		if(rtp_init(free_chn)<0){
			continue;
		}
		rtcp_init(free_chn);
		vist_type=rtspd_vtype(free_chn);
		if(!vist_type){
			send_file(free_chn);
		}
	}
	return 0;
}

int rtspd_loop(rtspd_call cb)
{
	if(!cb)
		return -1;

	//rtsp_cb=cb;
	cb();	
	return 0;
}

VOID *vd_rtspd_func(VOID *arg)
{
	pthread_detach(pthread_self());
	rtspd_loop(rtsp_callback);
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	CHAR version[32];
	signal(SIGINT, (VOID*)sig_exit);

	getrtspd_version(version);
	printf("rtspd version is %s\n",version);
		
	rtspd_init();
	rtsp_init("192.168.1.169" ,554);
	if (pthread_create(&rtspd_vthread, NULL, vd_rtspd_func,NULL) < 0){			
		printf("pthread_create rtcp error:\n");
		return -1;
	}	
	pause();
	printf("rtspd start release resources\n");
	rtsp_freeall();
	rtsp_free();
	rtspd_free();

	return 0;
}

