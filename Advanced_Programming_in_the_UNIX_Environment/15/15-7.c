#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXLINE 4096

int main(int argc,char * argv[])
{
	char line[MAXLINE];
	FILE *fpin;

	if((fpin=popen("15-6","r"))==NULL)
		printf("popen error!\n");
	for( ; ; )
	{
		fputs("prompt>",stdout);
		fflush(stdout);
		if(fgets(line,MAXLINE,fpin)==NULL)
			break;
		if(fputs(line,stdout)==EOF)
			printf("fputs error to pipe!\n");
	}
	if(pclose(fpin)==-1)
		printf("pclose error!\n");

	putchar('\n');
	exit(0);
}
