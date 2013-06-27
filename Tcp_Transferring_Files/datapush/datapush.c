#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_LENGTH 	(188*6)

int init_sock(char *des_ip, int port, int * sockfd, struct sockaddr_in *addr) {
    
    *sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd < 0) {
        printf("Socket Error:%s\n",strerror(errno));
        return -1;
    }

    bzero(addr,sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    if(inet_aton(des_ip,&addr->sin_addr) < 0) {
        printf("IP error:%s\n",strerror(errno));
        return -1;
    }

	return 0;
}

void datapush_loop(FILE *fp, size_t file_size, int sockfd, const struct sockaddr_in *addr) {
	size_t len = 0, total_len = 0;
	unsigned char buff[188*6];
	socklen_t addr_len = sizeof(struct sockaddr_in);

	while(1) {
		while(total_len < file_size) {
			len = fread(buff, 1, BUFFER_LENGTH, fp);
			total_len += len;
			if(sendto(sockfd, buff, len, 0, (struct sockaddr *)addr, addr_len) < 0) {
				printf("send data to server error!\n");
			}

			usleep(100);
			bzero(buff,BUFFER_LENGTH);	
		}

		fseek(fp, 0L, SEEK_SET);
		total_len = len = 0;
	}

	return;
}

void usage(void)
{
	printf("Usage: datapush [-i des_ip][-p des_port] [-f filename] [-h]\n");
    printf("        -i			destination ip address\n");
    printf("        -p          destination ip port\n");
    printf("        -f          file name\n");
    printf("        -h          help infomation\n");

	return;
}

int main(int argc, char **argv)
{
	int opt = 0;
	int des_port = 0;
	char des_ip[32];
	char file_name[32]; 
	FILE *fp = NULL;
	struct stat file_stat;
	int sockfd;
	struct sockaddr_in addr;
	
    while ((opt = getopt(argc, argv, "i:p:f:h")) != -1)
    {
        switch (opt)
        {
            case 'i':
				strcpy(des_ip,optarg);
                break;
            case 'p':
       			des_port = atoi(optarg);
                break;
			case 'f':
       			strcpy(file_name,optarg);
                break;
			case 'h':
				usage();
				break;
            default: /* '?' */
                break;
        }
    }

	fp = fopen(file_name, "r");
	if(fp == NULL) {
		printf("can not open %s\n", file_name);
		return -1;
	}
	
    stat(file_name, &file_stat);

	if(init_sock(des_ip, des_port, &sockfd, &addr) < 0) {
		printf("error: can not init socket!\n");
		return -1;
	}
	datapush_loop(fp, file_stat.st_size, sockfd, &addr);
	
	return 0;
}
