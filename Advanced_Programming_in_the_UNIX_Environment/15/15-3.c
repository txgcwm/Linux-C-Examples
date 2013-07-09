//#include <>

static int pfd1[2],pfd[2];

void TELL_WAIT(void)
{
	if(pipe(pfd1)||pipe(pfd2)<0)
		printf("creat pipe error!\n");
}

void TELL_PARENT(pid_t pid)
{
	if(write(pfd2[2],"c",1)!=1)
		printf("write error!\n");
}

void WAIT_PARENT(void)
{
	char c;
	if(read(pfd1[0],&c,1)!=1)
		printf("read error!\n");
	if(c!='p')
		printf("WAIT_PARENT:incorrect data!\n");
}

void TELL_CHILD(pid_t pid)
{
	if(write(pfd1[2],"p",1)!=1)
		printf("write error!\n");
}

void WAIT_CHILD(void)
{
	char c;
	if(read(pfd2[0],&c,1)!=1)
		printf("read error!\n");
	if(c!='c')
		printf("WAIT_CHILD:incorrect data!\n");
}
