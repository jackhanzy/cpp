#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc == 0) {
        fprintf(stdout, "usage ./tcp_client por\n");
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    char sendbuf[4096] = {'\0'};
    char recvbuf[4096] = {'\0'};
    while (fgets(sendbuf, 4096, stdin) != NULL) {
        write(sockfd, sendbuf, strlen(sendbuf));
        //sleep(1);
        //write(sockfd, sendbuf + 1, strlen(sendbuf) - 1);
        if (read(sockfd, recvbuf, 4096) == 0) {
            printf("read socket data failed\n");
            return -1;
        }
        fputs(recvbuf, stdout);
    }
    close(sockfd);
    return 0;
}
