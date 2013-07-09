#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>


#define PAGER "${PAGER:-more}"
#define MAXLINE 50

int main(int argc,char * argv[])
{
	char line[MAXLINE];
	FILE *fpin,*fpout;

	if(argc!=2)
		printf("usage:a.out <pathname>!\n");
	if((fpin=fopen(argv[1],"r"))==NULL)
		printf("can not open %s!\n",argv[1]);
	if((fpout=popen(PAGER,"w"))==NULL)
		printf("popen error");

	while(fgets(line,MAXLINE,fpin)!=NULL)
	{
		if(fputs(line,fpout)==EOF)
			printf("fputs error to pipe!\n");
	}

	if(ferror(fpin))
		printf("fgets error!\n");
	if(pclose(fpout)==-1)
		printf("pclose error!\n");

	exit(0);
}
