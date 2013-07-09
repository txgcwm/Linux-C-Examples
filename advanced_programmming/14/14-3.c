#include <fcntl.h>

pid_t lock_test(int fd,int type,off_t offset,int whence,off_t len)
{
	struct flock lock;
	lock.l_type=type;
	lock.l_start=offset;
	lock.l_whence=whence;
	lock.l_len=len;

	if(fcntl(fd,F_GETLK,&lock)<0)
		printf("fcntl error!");

	if(lock.l_type==F_UNLCK)
		return(0);

	return(lock.l_pid);
}

#define is_read_lockable(fd,offset,whence,len)\
	(lock_test((fd),F_RDLCK,(offset),(whence),(len))==0)

#define is_write_lockable(fd,offset,whence,len)\
	(lock_test((fd),F_WRLCK,(offset),(whence),(len))==0)