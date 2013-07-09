#include <stdio.h>
#include <unistd.h>

int main(int argc,char * argv[])
{
	if(chdir("/tmp")<0)
		printf("chdir failed!\n");
	else
		printf("chdir to /tmp successed!\n");

	_exit(0);
}
