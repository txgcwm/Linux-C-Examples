#include <setjmp.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

static void sig_int(int);
//void print_mask(const char *str);

volatile sig_atomic_t quitflag;

int main(int argc,char *argv[])
{
	sigset_t newmask,oldmask,zeromask;
	
	if(signal(SIGINT,sig_int)==SIG_ERR)
		printf("signal(SIGINT) error\n");
	if(signal(SIGQUIT,sig_int)==SIG_ERR)
		printf("signal(SIGQUIT) error\n");

	sigemptyset(&zeromask);
	sigemptyset(&newmask);
	sigaddset(&newmask,SIGQUIT);

	if(sigprocmask(SIG_BLOCK,&newmask,&oldmask)<0)
		printf("SIG_BLOCK error\n");

	while(quitflag==0)
		sigsuspend(&zeromask);

	quitflag=0;

	if(sigprocmask(SIG_SETMASK,&oldmask,NULL)<0)
		printf("SIG_SETMASK error\n");

	exit(0);
}


static void sig_int(int signo)
{
	if(signo==SIGINT)
		printf("\ninterrupt\n");
	else if(signo==SIGQUIT)
	{
		printf("\nwe have caught the SIGQUIT signal!\n");
		quitflag=1;
	}
}
