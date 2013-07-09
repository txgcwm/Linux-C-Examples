#include <setjmp.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

static void sig_usr1(int),sig_alrm(int);
void print_mask(const char *str);

static sigjmp_buf   jmpbuf;
static volatile sig_atomic_t   canjump;


int main(int argc,char *argv[])
{
	if(signal(SIGUSR1,sig_usr1)==SIG_ERR)
		printf("signal(SIGUSR1) error\n");
	if(signal(SIGALRM,sig_alrm)==SIG_ERR)
		printf("signal(SIGALRM) error\n");

	print_mask("starting main:\n");

	if(sigsetjmp(jmpbuf,1))
	{
		print_mask("ending main:\n");
		exit(0);
	}
	canjump=1;

	for(; ;)
	   pause();	

}

static void sig_usr1(int signo)
{
	time_t  starttime;

	if(canjump==0)
		return;

	print_mask("starting sig_usr1:\n");
	alarm(3);
	starttime=time(NULL);

	for(; ;)
         if(time(NULL)>starttime+5)
		break;

	print_mask("finishing sig_usr1:\n");

	canjump=0;
	siglongjmp(jmpbuf,1);
}

static void sig_alrm(int signo)
{
	print_mask("in sig_alrm:\n");
}

void print_mask(const char *str)
{
	sigset_t sigset;
	int errno_save;

	errno_save=errno;

	if(sigprocmask(0,NULL,&sigset)<0)
		printf("sigprocmask error!\n");

	printf("%s",str);

	if(sigismember(&sigset,SIGINT))  printf("SIGINT\n");
	if(sigismember(&sigset,SIGQUIT))  printf("SIGQUIT\n");
	if(sigismember(&sigset,SIGUSR1))  printf("SIGUSR1\n");
	if(sigismember(&sigset,SIGALRM))  printf("SIGALRM\n");

	printf("\n");
	errno=errno_save;
}
