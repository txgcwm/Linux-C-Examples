#include <termios.h>
#ifndef TIOCGWINSZ
	#include <sys/ioctl.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

static void pr_winsize(int fd)
{
	struct winsize size;

	if(ioctl(fd,TIOCGWINSZ,(char *)&size)<0)
		printf("TIOCGWINSZ error!\n");

	printf("%d rows,%d columns\n",size.ws_row,size.ws_col);
}

static void sig_winch(int signo)
{
	printf("SIGWINCH received!\n");
	pr_winsize(STDIN_FILENO);
}

int main(void)
{
	if(isatty(STDIN_FILENO)==0)
		exit(1);

	if(signal(SIGWINCH,sig_winch)==SIG_ERR)
		printf("signal error!\n");

	pr_winsize(STDIN_FILENO);
	for( ; ; )
		pause();
}
