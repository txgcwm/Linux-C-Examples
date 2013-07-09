#include <stdio.h>
#include <stdlib.h>

#define MAXLINE 50

int main(int argc,char * argv[])
{
	int int1,int2;
	char line[MAXLINE];

	while(fgets(line,MAXLINE,stdin)!=NULL)
	{
		if(sscanf(line,"%d%d",&int1,&int2)==2)
		{
			if(printf("%d\n",int1+int2)==EOF)
				printf("printf error!\n");
		}
		else
		{
			if(printf("invalid args!\n")==EOF)
				printf("printf error!\n");
		}
	}
	exit(0);
}
