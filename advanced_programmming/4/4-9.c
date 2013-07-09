#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc,char * argv[])
{
	char *ptr;
	int size=50;

	if(chdir("/home/min.wu/linux_study")<0)
		printf("chdir failed!\n");
	else
		printf("chdir to /home/min.wu/linux_study successed!\n");

	ptr=malloc(size);

	if(getcwd(ptr,size)==NULL)
		printf("try to getcwd error!\n");
	else
		printf("cwd = %s\n",ptr);

	_exit(0);
}
