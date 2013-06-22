#include "common/common.h"

int main(int argc, char *argv[])
{
	int sockfd;
    char buff[BUFFERSIZE + 1];
    char filenameheader[BUFFERSIZE + 1];   
    int filefd;    /* file descriptor */
    int count;
	struct sockaddr_in servaddr;

    if (argc != 3) {
        fprintf(stderr, "Usage: ./fileclient <file> <serverIP>\n");
        exit(1);
    }

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    Inet_pton(AF_INET, argv[2], &servaddr.sin_addr);
    servaddr.sin_port = htons(PORT);

    printf("connecting........\n");
    Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    printf("transferring file name: %s........\n", argv[1]);
    memcpy(filenameheader, argv[1], strlen(argv[1]));
    filenameheader[strlen(argv[1])] = '\n';
    filenameheader[strlen(argv[1]) + 1] = 0;
    writen(sockfd, filenameheader, strlen(filenameheader));

    printf("will transfer file: %s\n", argv[1]);

    filefd = open(argv[1], O_RDONLY);
    if (filefd < 0) {
        fprintf(stderr, "can't open the file: %s\n", argv[1]);
        exit(1);
    }

    while(count = read(filefd, buff, BUFFERSIZE)) {
        if (count < 0) {
            fprintf(stderr, "filefd read error\n");
            exit(1);
        }
        if (writen(sockfd, buff, count) < 0) {
            fprintf(stderr, "writing to sockfd error\n");
            exit(1);
        }
    }

    Close(filefd);
    Close(sockfd);

    printf("file %s transferred!\n", argv[1]);

    return 0;
}
