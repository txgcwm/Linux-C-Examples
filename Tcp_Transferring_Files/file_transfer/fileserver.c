#include "common/common.h"

int main(int argc, char *argv[])
{
    int listenfd, connfd;
    char buff[BUFFERSIZE + 1];
    char filename[BUFFERSIZE + 1]; 
    int cliaddrlen;
    int filefd;    /* file descriptor */
    int count;
	struct sockaddr_in servaddr, cliaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    Listen(listenfd, LISTENQ);

    while(1) {
        printf("listening........\n");

        cliaddrlen = sizeof(cliaddr);
        connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddrlen);
        printf("readinging filename........\n");
        if (readline(connfd, buff, BUFFERSIZE) < 0) {
            fprintf(stderr, "readline error\n");
            exit(1);
        }
        buff[strlen(buff) - 1] = 0;    /* change '\n' to NUL */
        memcpy(filename, buff, BUFFERSIZE + 1);
        printf("will save to file: %s\n", buff);

        filefd = open(buff, O_WRONLY | O_CREAT);
        if (filefd < 0) {
            fprintf(stderr, "can't open the file: %s\n", buff);
            exit(1);
        }

        while(count = read(connfd, buff, BUFFERSIZE)) {
            if (count < 0) {
                fprintf(stderr, "connfd read error\n");
                exit(1);
            }
            if (writen(filefd, buff, count) < 0) {
                fprintf(stderr, "writing to filefd error\n");
                exit(1);
            }
        }

        Close(filefd);
        Close(connfd);
        printf("file %s received!\n", filename);
    }

    Close(listenfd);

    return 0;
}
