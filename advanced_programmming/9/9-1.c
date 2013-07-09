#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

static void sig_hup(int signo)
{
	printf("SIGHUP received,pid = %d\n",getpid());
}

static void print_id(char *name)
{
	printf("%s:pid = %d ,ppid = %d ,pgrp = %d ,tpgrp = %d\n",
		name,getpid(),getppid(),getpgrp(),tcgetpgrp(STDIN_FILENO));
	fflush(stdout);
}

int main(int argc,char * argv[])
{
	char c;
	pid_t pid;

	print_id("parent");
	if((pid=fork())<0)
		printf("fork error!\n");
	else if(pid>0)
	{
		sleep(5);
		exit(0);
	}
	else
	{
		print_id("child");
		signal(SIGHUP,sig_hup);
		kill(getpid(),SIGTSTP);
		print_id("child");

		if(read(STDIN_FILENO,&c,1)!=1)
			printf("read error from controlling TTY,errno=%d\n",errno);

		exit(0);
	}
}
