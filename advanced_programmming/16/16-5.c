#include <netdb.h>
#include <errno.h>
#include <syslog.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <signal.h>

#define BUFLEN 128
#define QLEN 10

#ifndef HOST_NAME_MAX
	#define HOST_NAME_MAX 256
#endif

void daemonize(const char *cmd)
{
        int  i,fd0,fd1,fd2;
        pid_t pid;
        struct rlimit   r1;
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

int initserver(int type,const struct sockaddr *addr,socklen_t alen,int qlen)
{
        int fd;
        int err=0;

        if((fd=socket(addr->sa_family,type,0))<0)
                return(-1);
        if(bind(fd,addr,alen)<0)
        {
                err=errno;
                goto errout;
        }

        if(type==SOCK_STREAM||type==SOCK_SEQPACKET)
        {
                if(listen(fd,qlen)<0)
                {
                        err=errno;
                        goto errout;
                }
        }
        return(fd);

    errout:
        close(fd);
        errno=err;
        return(-1);
}

void serve(int sockfd)
{
	int clfd;
	FILE *fp;
	char buf[BUFLEN];

	for( ; ; )
	{
		clfd=accept(sockfd,NULL,NULL);
		if(clfd<0)
		{
			syslog(LOG_ERR,"ruptimed:accept error :%s",strerror(errno));
			exit(1);
		}

		if((fp=popen("/usr/bin/uptime","r"))==NULL)
		{
			sprintf(buf,"error:%s\n",strerror(errno));
			send(clfd,buf,strlen(buf),0);
		}
		else
		{
			while(fgets(buf,BUFLEN,fp)!=NULL)
				send(clfd,buf,strlen(buf),0);
			pclose(fp);
		}

		close(clfd);
	}
}

int main(int argc,char *argv[])
{
	struct addrinfo *ailist,*aip;
       struct addrinfo hint;
       int sockfd,err,n;
	char *host;

       if(argc!=1)
             printf("usage:ruptimed!\n");

#ifdef _SC_HOST_NAME_MAX
	n=sysconf(_SC_HOST_NAME_MAX);
	if(n<0)
#endif
	n=HOST_NAME_MAX;

	host=malloc(n);
	if(host==NULL)
		printf("malloc error!\n");
	if(gethostname(host,n)<0)
		printf("gethostname error!\n");
	daemonize("ruptime");

       hint.ai_flags=AI_CANONNAME;
       hint.ai_family=0;
       hint.ai_socktype=SOCK_STREAM;
       hint.ai_protocol=0;
       hint.ai_addrlen=0;
       hint.ai_canonname=NULL;
       hint.ai_addr=NULL;
       hint.ai_next=NULL;

	if((err=getaddrinfo(host,"ruptime",&hint,&ailist))!=0)
	{
		syslog(LOG_ERR,"ruptimed:getaddrinfo error:%s\n",gai_strerror(err));
		exit(1);
	}

	for(aip=ailist;aip!=NULL;aip=aip->ai_next)
	{
		if((sockfd=initserver(SOCK_STREAM,aip->ai_addr,aip->ai_addrlen,QLEN))>=0)
			serve(sockfd);
		exit(0);
	}

	exit(1);
}


