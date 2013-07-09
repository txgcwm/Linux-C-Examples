#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

static void sig_alrm(int signo)
{

}

unsigned int sleep1(unsigned int nseces)
{
	if(signal(SIGALRM,sig_alrm)==SIG_ERR)
		return(nsecs);

	alarm(nsecs);
	pause();
	
	return(alarm(0));	
}

