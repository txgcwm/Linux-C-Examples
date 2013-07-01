#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define PORT 		8888
#define SERVER_IP 	"127.0.0.1"
#define BUFFER_SIZE 1024

int random_data(void)
{
    srand((int) time(NULL));

    return (rand()%(512) + 512);
}

ssize_t sendn(int sock_fd, const void *ptr, size_t n, int flags)
{
    size_t nleft = n;
    ssize_t nwritten;

    while(nleft > 0) {
        if((nwritten = send(sock_fd, ptr, nleft, flags)) < 0) {
            if(nleft == n)
                return -1;
            else
                break;
        } else if(nwritten == 0) 
            break;

        nleft -= nwritten;
        ptr += nwritten;
    }

    return (n - nleft);
}

int main(int argc, char **argv)
{
    int s;
    struct sockaddr_in addr;
    char buffer[BUFFER_SIZE];
	int fd = -1;
	ssize_t size = 0;
	ssize_t nsendsize = 0;

	fd = open("video_cmmb.h264",O_RDONLY); 
	if(fd < 0) {
		perror("open file error!\n");
		return -1;
	}

    if((s = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        perror("socket");
       	return -1;
    }
  
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(SERVER_IP);
 
    if(connect(s,(struct sockaddr *)&addr,sizeof(addr))<0) {
        perror("connect");
		return -1;
    }
  
    recv(s,buffer,sizeof(buffer),0);
    printf("%s\n",buffer);

    while(1) {
        bzero(buffer, BUFFER_SIZE);

        size = read(fd, buffer, random_data());
		if(size <= 0)
			break;

        if((nsendsize = sendn(s, buffer, size, 0)) < 0) {
		    perror("sendn error!\n");
		    lseek(fd, -size, SEEK_CUR);
		    return -1;
		}

		if(nsendsize < size)
		    lseek(fd, nsendsize - size, SEEK_CUR);

		usleep(1000);
    }

	return 0;
}
