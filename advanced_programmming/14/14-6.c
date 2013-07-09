#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

static volatile sig_atomic_t sigflag;
static sigset_t newmask,oldmask,zeromask;

static void sig_usr(int signo)
{
        sigflag=1;
}

void TELL_WAIT(void)
{
        if(signal(SIGUSR1,sig_usr)==SIG_ERR)
                printf("signal(SIGUSR1)error\n");
        if(signal(SIGUSR2,sig_usr)==SIG_ERR)
                printf("signal(SIGUSR2)error\n");

        sigemptyset(&zeromask);
        sigemptyset(&newmask);
        sigaddset(&newmask,SIGUSR1);
        sigaddset(&newmask,SIGUSR1);

        if(sigprocmask(SIG_BLOCK,&newmask,&oldmask)<0)
                printf("SIG_BLOCK error\n");
}

void TELL_PARENT(pid_t pid)
{
        kill(pid,SIGUSR2);
}

void WAIT_PARENT(void)
{
        while(sigflag==0)
                sigsuspend(&zeromask);
        sigflag=0;
        if(sigprocmask(SIG_SETMASK,&oldmask,NULL)<0)
                printf("SIG_SETMASK error\n");
}

void TELL_CHILD(pid_t pid)
{
        kill(pid,SIGUSR2);
}

void WAIT_CHILD(void)
{
        while(sigflag==0)
                sigsuspend(&zeromask);
        sigflag=0;
        if(sigprocmask(SIG_SETMASK,&oldmask,NULL)<0)
                printf("SIG_SETMASK error\n");

}

void set_fl(int fd,int flags)
{
        int val;

        if((val=fcntl(fd,F_GETFL,0))<0)
                printf("fcntl F_GETFL error");

        val |=flags;

        if(fcntl(fd,F_SETFL,val)<0)
                printf("fcntl F_SETFL error");
}

int lock_reg(int fd,int cmd,int type,off_t offset,int whence,off_t len)
{
        struct flock lock;

        lock.l_type=type;
        lock.l_start=offset;
        lock.l_whence=whence;
        lock.l_len=len;

        return(fcntl(fd,cmd,&lock));
}

#define read_lock(fd,offset,whence,len)\
        lock_reg((fd),F_SETLK,F_RDLCK,(offset),(whence),(len))

#define readw_lock(fd,offset,whence,len)\
        lock_reg((fd),F_SETLKW,F_RDLCK,(offset),(whence),(len))

#define write_lock(fd,offset,whence,len)\
        lock_reg((fd),F_SETLK,F_WRLCK,(offset),(whence),(len))

#define writew_lock(fd,offset,whence,len)\
        lock_reg((fd),F_SETLKW,F_WRLCK,(offset),(whence),(len))

#define un_lock(fd,offset,whence,len)\
        lock_reg((fd),F_SETLK,F_UNLCK,(offset),(whence),(len))

int main(int argc,char *argv[])
{
	int fd;
	pid_t pid;
	char buf[5];
	struct stat statbuf;

	if(argc!=2)
	{
		fprintf(stderr,"usage:%s filename\n",argv[0]);
		exit(1);
	}

	if((fd=open(argv[1],O_RDWR|O_CREAT|O_TRUNC,FILE_MODE))<0)
		printf("open error\n");

	if(write(fd,"abcdef",6)!=6)
		printf("write error!\n");

	if(fstat(fd,&statbuf)<0)
             printf("get the stat error\n");
       if(fchmod(fd,(statbuf.st_mode&~S_IXGRP)|S_ISGID)<0)
             printf("chmod error for foo!\n");

	TELL_WAIT();

	 if((pid=fork())<0)
                printf("fork error!\n");
        else if(pid>0)
        {
        	if(write_lock(fd,0,SEEK_SET,0)<0)
			printf("write_lock error\n");

		TELL_CHILD(pid);
		
		sleep(5);
		if(waitpid(pid,NULL,0)<0)
			printf("waitpid error!\n");
        }
        else
        {
              WAIT_PARENT();
		set_fl(fd,O_NONBLOCK);

		if(read_lock(fd,0,SEEK_SET,0)!=-1)
			printf("child:read_lock succeeded\n");
		printf("read_lock of already-locked region returns %d\n",errno);

		if(lseek(fd,0,SEEK_SET)==-1)
			printf("lseek error!\n");
		if(read(fd,buf,2)<0)
			printf("read failed (mandatory locking works)");
		else
			printf("read OK (no mandatory locking),buf=%2.2s\n",buf);           
        }

	exit(0);
}
