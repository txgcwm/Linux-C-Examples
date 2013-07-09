#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>

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

static void lockabyte(const char *name,int fd,off_t offset)
{
	if(writew_lock(fd,offset,SEEK_SET,1)<0)
	{
		printf("%s,writew_lock error\n",name);
		return;
	}

	printf("%s:got the lock,byte %ld\n",name,offset);
}

int main(void)
{
	int fd;
	pid_t pid;

	if((fd=creat("templock",FILE_MODE))<0)
		printf("creat error");
	if(write(fd,"ab",2)!=2)
		printf("write error");

	TELL_WAIT();

	if((pid=fork())<0)
		printf("fork error!");
	else if(pid==0)
	{
		sleep(2);
		lockabyte("child",fd,0);
		TELL_PARENT(getppid());
		WAIT_PARENT();
		lockabyte("child",fd,1);
	}
	else
	{
		lockabyte("parent",fd,1);
		TELL_PARENT(pid);
		WAIT_PARENT();
		lockabyte("parent",fd,0);
	}
}
