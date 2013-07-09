#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

typedef void sigfunc(int);
sigfunc *signal(int,sigfunc *);

static void sig_usr(int signo)
{
	if(signo==SIGUSR1)
		printf("received SIGUSR1\n");
	else if(signo==SIGUSR2)
		printf("received SIGUSR2\n");
	else
		printf("received signal %d \n",signo);
}

int main(int argc,char * argv[])
{
	if(signal(SIGUSR1,sig_usr)==SIG_ERR)
		printf("can not catch SIGUSR1!\n");
	if(signal(SIGUSR2,sig_usr)==SIG_ERR)
		printf("can not catch SIGUSR2!\n");

	for( ; ; )
	pause();

}
