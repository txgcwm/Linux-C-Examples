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


#define SERVER_IP 	"127.0.0.1"
#define PORT 		8888
#define BUFFER_SIZE	1024

int random_data(void)
{
    srand((int) time(NULL));

    return (rand()%(512) + 512);
}

int main(int argc, char **argv)
{
    int s;
    struct sockaddr_in addr;
    char buffer[BUFFER_SIZE];
	int fd = -1;
	ssize_t size = 0;

	fd = open("video_cmmb.h264",O_RDONLY); 
	if(fd < 0) {
		perror("open file error!\n");
		return -1;
	}

    if((s = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        perror("socket");
        exit(1);
    }
  
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(SERVER_IP);
 
    if(connect(s,(struct sockaddr *)&addr,sizeof(addr))<0) {
        perror("connect");
        exit(1);
    }
  
    recv(s,buffer,sizeof(buffer),0);
    printf("%s\n",buffer);

    while(1) {
        bzero(buffer, BUFFER_SIZE);

        size = read(fd, buffer, random_data());
		if(size <= 0)
			break;

        if(send(s, buffer, size, 0) < 0) {
            perror("send");
            exit(1);
        }

		usleep(1000*10);
    }

	return 0;
}
