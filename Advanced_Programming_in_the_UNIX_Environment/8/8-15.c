#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


int main(int argc,char *argv[])
{
	printf("real uid=%d,effective uid=%d\n",getuid(),geteuid());

	exit(0);
}




