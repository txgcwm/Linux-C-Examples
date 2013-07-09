#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/times.h>

static void print_times(clock_t real,struct tms *tmsstart,struct tms *tmsend);
void print_exit(int status);
static void do_cmd(char *cmd);

int main(int argc,char *argv[])
{
	int i;

	setbuf(stdout,NULL);
	for(i=1;i<argc;i++)
		do_cmd(argv[i]);
	exit(6);
}

static void do_cmd(char *cmd)
{
	struct tms tmsstart,tmsend;
	clock_t start,end;
	int status;

	printf("\ncommand:%s !\n",cmd);

	if((start=times(&tmsstart))==-1)
		printf("times error!\n");

	if((status=system(cmd))<0)
		printf("system error!\n");

	if((end=times(&tmsend))==-1)
		printf("time error!\n");

	print_times(end-start,&tmsstart,&tmsend);
	print_exit(status);
}

static void print_times(clock_t real,struct tms *tmsstart,struct tms *tmsend)
{
	static long clktck=0;

	if(clktck==0)
		if((clktck=sysconf(_SC_CLK_TCK)<0))
			printf("sysconf error!\n");

	printf("real: %7.2f\n",real/(double)clktck);
	printf("user: %7.2f\n",(tmsend->tms_utime-tmsstart->tms_utime)/(double)clktck);
	printf("sys: %7.2f\n",(tmsend->tms_stime-tmsstart->tms_stime)/(double)clktck);
	printf("child user: %7.2f\n",(tmsend->tms_cutime-tmsstart->tms_cutime)/(double)clktck);
	printf("child sys: %7.2f\n",(tmsend->tms_cstime-tmsstart->tms_cstime)/(double)clktck);
}

void print_exit(int status)
{
	if(WIFEXITED(status))
		printf("normal termination,exit status = %d \n",WEXITSTATUS(status));
	else if(WIFSIGNALED(status))
		printf("abnormal termination,signal number =%d %s\n",WTERMSIG(status),
			#ifdef WCOREDUMP
				WCOREDUMP(status)? "(core file generated)":"");
			#else
				"");
			#endif
	else if(WIFSTOPPED(status))
		printf("child stopped,signal number = %d\n",WSTOPSIG(status));
}


