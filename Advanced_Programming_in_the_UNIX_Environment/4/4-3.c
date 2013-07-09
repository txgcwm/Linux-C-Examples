#include <stdio.h>
#include <fcntl.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define RWRWRW (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

int main(int argc,char * argv[])
{
	umask(0);

	if(creat("foo",RWRWRW)<0)
		printf("create foo error,please check it!\n");

	umask(S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

	if(creat("bar",RWRWRW)<0)
		printf("create bar error,please check it!\n");

	_exit(0);
}

