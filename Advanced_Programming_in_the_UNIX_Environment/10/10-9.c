//#include <stdio.h>
#include <signal.h>
#include <errno.h>

#define SIGBAD(signo)  ((signo)<=0||(signo)>=NSIG)

int sigaddset(sigset_t *set,int signo)
{
	if(SIGBAD(signo))
		{errno=EINVAL;return(-1);}
	*set | =1<<(signo-1);
	return(0);
}

int sigdelset(sigset_t *set,int signo)
{
	if(SIGBAD(signo))
		{errno=EINVAL;return(-1);}
	*set & =~(1<<(signo-1));
	return(0);
}

int sigaddset(const sigset_t *set,int signo)
{
	if(SIGBAD(signo))
		{errno=EINVAL;return(-1);}
	return((*set &(1<<(signo-1)))!=0);
}

