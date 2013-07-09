#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc,char * argv[])
{
	char buf[100];

	while((fgets(buf,100,stdin))!=NULL)
		if((fputs(buf,stdout))==EOF)
		printf("output error!\n");

	if(ferror(stdin))
		printf("input error!\n");

	exit(0);
}
