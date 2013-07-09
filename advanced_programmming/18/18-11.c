#include <termios.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static struct termios save_termios;
static int ttysavefd=-1;
static enum{RESET,RAW,CBREAK}  ttystate=RESET;

int tty_cbreak(int fd)
{
	int err;
	struct termios buf;

	if(ttystate!=RESET)
	{
		errno=EINVAL;
		return(-1);
	}

	if(tcgetattr(fd,&buf)<0)
		return(-1);

	save_termios=buf;

	buf.c_lflag &=~(ECHO | ICANON);
	buf.c_cc[VMIN]=1;
	buf.c_cc[VTIME]=0;
	if(tcsetattr(fd,TCSAFLUSH,&buf)<0)
		return(-1);

	if(tcgetattr(fd,&buf)<0)
	{
		err=errno;
		tcsetattr(fd,TCSAFLUSH,&save_termios);
		errno=err;
		return(-1);
	}

	if((buf.c_lflag & (ECHO | ICANON))||buf.c_cc[VMIN]!=1 || buf.c_cc[VTIME]!=0)
	{
		tcsetattr(fd,TCSAFLUSH,&save_termios);
		errno=EINVAL;
		return(-1);
	}

	ttystate=CBREAK;
	ttysavefd=fd;
	return(0);
}

int tty_raw(int fd)
{
	int  err;
	struct termios buf;

	if(ttystate!=RESET)
	{
		errno=EINVAL;
		return(-1);
	}

	if(tcgetattr(fd,&buf)<0)
		return(-1);
	save_termios=buf;

	buf.c_lflag &=~(ECHO | ICANON |IEXTEN | ISIG);

	buf.c_iflag &=~(BRKINT | ICRNL | INPCK | ISTRIP |IXON);

	buf.c_cflag &=~(CSIZE | PARENB);
	
	buf.c_cflag |=CS8;

	buf.c_oflag &=~(OPOST);

	buf.c_cc[VMIN]=1;
	buf.c_cc[VTIME]=0;

	if(tcsetattr(fd,TCSAFLUSH,&buf)<0)
		return(-1);

	if(tcgetattr(fd,&buf)<0)
	{
		err=errno;
		tcsetattr(fd,TCSAFLUSH,&save_termios);
		errno=err;
		return(-1);
	}

	if((buf.c_lflag & (ECHO | ICANON |IEXTEN | ISIG))||
		(buf.c_iflag & (BRKINT | ICRNL | INPCK | ISTRIP |IXON))||
		(buf.c_cflag & (CSIZE | PARENB | CS8)!=CS8)||
		(buf.c_oflag & OPOST)||buf.c_cc[VMIN]!=1 || buf.c_cc[VTIME]!=0)
	{
		tcsetattr(fd,TCSAFLUSH,&save_termios);
		errno=EINVAL;
		return(-1);
	}
	
	ttystate=RAW;
	ttysavefd=fd;
	return(0);
}

int tty_reset(int fd)
{
	if(ttystate==RESET)
		return(0);
	if(tcsetattr(fd,TCSAFLUSH,&save_termios)<0)
		return(-1);
	ttystate=RESET;
	return(0);
}

void tty_atexit(void)
{
	if(ttysavefd>=0)
		tty_reset(ttysavefd);
}

struct termios * tty_termios(void)
{
	return(&save_termios);
}



static void sig_catch(int signo)
{
	printf("signal caught\n");
	tty_reset(STDIN_FILENO);
	exit(0);
}

int main(void)
{
	int i;
	char c;

	if(signal(SIGINT,sig_catch)==SIG_ERR)
		printf("signal (SIGINT) error!\n");
	if(signal(SIGQUIT,sig_catch)==SIG_ERR)
		printf("signal (SIGQUIT) error!\n");
	if(signal(SIGTERM,sig_catch)==SIG_ERR)
		printf("signal (SIGTERM) error!\n");

	if(tty_raw(STDIN_FILENO)<0)
		printf("tty_raw error!\n");
	printf("Enter raw mode characters,terminate with DELETE\n");

	while((i=read(STDIN_FILENO,&c,1))==1)
	{
		if((c &=255)==0177)
			break;
		printf("%o\n",c);
	}
	if(tty_reset(STDIN_FILENO)<0)
		printf("tty_reset error!\n");
	if(i<=0)
		printf("read error!\n");
	if(tty_cbreak(STDIN_FILENO)<0)
		printf("tty_cbreak error!\n");
	printf("\nEnter cbreak mode characters,terminate with SIGINT\n");

	while((i=read(STDIN_FILENO,&c,1))==1)
	{
		c &=255;
		printf("%o\n",c);
	}
	if(tty_reset(STDIN_FILENO)<0)
		printf("tty_reset error!\n");

	if(i<=0)
		printf("read error!\n");

	exit(0);
}

















