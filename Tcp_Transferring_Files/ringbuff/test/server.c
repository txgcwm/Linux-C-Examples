#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "ringbuff.h"

#define PORT 		8888
#define MAXSOCKFD 	10
#define BUFFER_SIZE 1024

ring_buff_p ringbuff = NULL;

void *write_data2file(void *arg)
{	
	unsigned char buffer[BUFFER_SIZE];
	int size = 0;
	int fd = -1;

	printf("start \"%s\" pthread  %d\n", __FUNCTION__, __LINE__);

	fd = open("test.h264", O_RDWR|O_CREAT, S_IRUSR | S_IWUSR); 
	if(fd < 0) {
		perror("open file error!\n");
		return NULL;
	}

	while(1){
		if(!ringbuff_data_size(ringbuff)){
			usleep(10);
			continue;
		}

		size = ringbuff_read(ringbuff, buffer, BUFFER_SIZE);
	
		write(fd, buffer, size);
	}

	return NULL;
}
    
int main(int argc, char **argv)
{
    int sockfd,newsockfd,is_connected[MAXSOCKFD],fd;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    fd_set readfds;
    char buffer[BUFFER_SIZE];
    char msg[] ="Welcome to server!";
	ssize_t size = 0;
	pthread_t pid;

	ringbuff_init(&ringbuff, 20*1024);

    pthread_create(&pid, NULL, write_data2file, NULL);

    if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        perror("socket");
        exit(1);
    }

    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd,(struct sockaddr *)&addr,sizeof(addr)) < 0) {
        perror("connect");
        exit(1);
    }

    if(listen(sockfd,3) < 0) {
        perror("listen");
        exit(1);
    }

    for(fd=0; fd<MAXSOCKFD; fd++)
        is_connected[fd] = 0;

    while(1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd,&readfds);
 
        for(fd=0; fd<MAXSOCKFD; fd++) {
            if(is_connected[fd]) 
				FD_SET(fd,&readfds);
		}
        if(!select(MAXSOCKFD,&readfds,NULL,NULL,NULL))
			continue;

        for(fd=0; fd<MAXSOCKFD; fd++) {
            if(FD_ISSET(fd,&readfds)) {
                if(sockfd == fd) {
                    if((newsockfd = accept(sockfd,(struct sockaddr *)&addr,&addr_len)) < 0)
                        perror("accept");
                    write(newsockfd,msg,sizeof(msg));
                    is_connected[newsockfd] = 1;
                    printf("cnnect from %s\n",inet_ntoa(addr.sin_addr));
                } else {
                    bzero(buffer, BUFFER_SIZE);

                    if((size = read(fd, buffer, BUFFER_SIZE)) <= 0) {
                        printf("connect closed.\n");
                        is_connected[fd] = 0;
                        close(fd);
                    } else {
						ringbuff_write(ringbuff, buffer, size);
					}
                }
            }
		}
    }

	ringbuff_close(ringbuff);

	return 0;
}
