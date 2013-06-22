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

#if (defined LIST_BUF)
	#include "list_buf.h"
	struct list_head head;
#elif (defined LOOP_BUF)
	#include "loop_buf.h"
	struct loop_buf buf;
#endif

#define PORT 		8888
#define MAXSOCKFD 	10
#define BUFFER_SIZE 1024


void *write_data2file(void *arg)
{	
	int fd = -1;

	printf("start \"%s\" pthread  %d\n", __FUNCTION__, __LINE__);

	fd = open("test.h264", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); 
	if(fd < 0) {
		perror("open file error!\n");
		return NULL;
	}

#if (defined LIST_BUF)
	while (1) {
		struct h264_raw *entry = NULL;

		if (list_empty(&head)) {
			usleep(10);
			continue;
		}
		
		entry = list_first_entry(&head, struct h264_raw, list);
		if(entry != NULL) {
			read_data(entry, 325, fd);
		}		
	}
#elif (defined LOOP_BUF)
	unsigned char buffer[BUFFER_SIZE];
	int size = 0;

	while(1){
		if(!loop_buf_len(&buf)){
			usleep(10);
			continue;
		}

		lock_buf(&buf);
		size = get_loop_buf(&buf, buffer, BUFFER_SIZE);
		unlock_buf(&buf);
	
		if(write(fd, buffer, size) < 0) {
			printf("write data fail!\n");
		}
	}
#endif

	return NULL;
}
    
int main(int argc, char **argv)
{
    int sockfd,newsockfd,is_connected[MAXSOCKFD],fd;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    fd_set readfds;
    unsigned char buffer[BUFFER_SIZE];
    char msg[] ="Welcome to server!";
	ssize_t size = 0;
	pthread_t pid;

#if (defined LIST_BUF)
	list_buf_init(&head);
#elif (defined LOOP_BUF)
	init_loop_buf(&buf);
#endif

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
                    if(write(newsockfd,msg,sizeof(msg)) < 0){
                    	printf("write data fail\n");
                    }
                    is_connected[newsockfd] = 1;
                    printf("cnnect from %s\n",inet_ntoa(addr.sin_addr));
                } else {
                    bzero(buffer, BUFFER_SIZE);

                    if((size = read(fd, buffer, BUFFER_SIZE)) <= 0) {
                        printf("connect closed.\n");
                        is_connected[fd] = 0;
                        close(fd);
                    } else {
                    #if (defined LIST_BUF)
						add_data_to_list(buffer, size, &head);
					#elif (defined LOOP_BUF)
						lock_buf(&buf);
						put_loop_buf(&buf, buffer, size);
						unlock_buf(&buf);						
					#endif
					}
                }
            }
		}
    }

#if (defined LOOP_BUF)
    release_loop_buf(&buf);
#endif
	return 0;
}
