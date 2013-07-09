#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>


void daemonize(const char *cmd);

int main(int argc,char * argv[])
{
	daemonize("ls");
	//sleep(100);
	pause();
	return 0;	
}

void daemonize(const char *cmd)
{
	int  i,fd0,fd1,fd2;
	pid_t pid;
	struct rlimit	r1;
	struct sigaction sa;

	umask(0);
	if(getrlimit(RLIMIT_NOFILE,&r1)<0)
		printf("%s:can not get file limit",cmd);

	if((pid=fork())<0)
		printf("%s:can not fork",cmd);
	else if(pid!=0)
		exit(0);

	setsid();
	sa.sa_handler=SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags=0;
	if(sigaction(SIGHUP,&sa,NULL)<0)
		printf("%s:can not ignore SIGHUP!\n",cmd);
	/*if((pid=fork())<0)
		printf("%s:can not fork!\n",cmd);
	else if(pid!=0)
		exit(0);*/

	if(chdir("/")<0)
		printf("can not change directory to /\n");

	if(r1.rlim_max==RLIM_INFINITY)
		r1.rlim_max=1024;
	for(i=0;i<r1.rlim_max;i++)
		close(i);

	fd0=open("/dev/null",O_RDWR);
	fd1=dup(0);
	fd2=dup(0);

	openlog(cmd,LOG_CONS,LOG_DAEMON);
	if(fd0!=0||fd1!=1||fd2!=2)
	{
		syslog(LOG_ERR,"unexpected file descriptors %d %d %d",fd0,fd1,fd2);
		exit(1);
	}
}
