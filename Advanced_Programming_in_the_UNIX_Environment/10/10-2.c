#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pwd.h>

typedef void sigfunc(int);
sigfunc *signal(int,sigfunc *);

static void my_alarm(int signo)
{
	struct passwd *rootptr;

	printf("in signal handler!\n");
	if((rootptr=getpwnam("root"))==NULL)
		printf("getpwnam(root) error!\n");

	alarm(1);
}

int main(int argc,char * argv[])
{
	struct passwd *ptr;
	signal(SIGALRM,my_alarm);

	for( ; ; )
	{
		if((ptr=getpwnam("min.wu"))==NULL)
			printf("getpwnam error!\n");
		if(strcmp(ptr->pw_name,"min.wu")!=0)
			printf("return value corrupted!,pw_name = %s \n",ptr->pw_name);
	}
}


