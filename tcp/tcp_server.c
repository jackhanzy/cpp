#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

void sig_child(int signo);
int main() {
    int svr_fd, connfd;
    struct sockaddr_in serv_addr, cli_addr;
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    //inet_aton(("172.17.213.214"), &serv_addr.sin_addr);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(9527);
    svr_fd = socket(AF_INET, SOCK_STREAM, 0);
    bind(svr_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(svr_fd, 0);
    struct sockaddr_in local_addr;
    socklen_t local_len = sizeof(local_addr);
    bzero((char*)&local_addr, sizeof(local_addr));
    if (getsockname(svr_fd, (struct sockaddr*)&local_addr, &local_len) == 0) {
        char local_ip[20];
        inet_ntop(AF_INET, &local_addr.sin_addr, local_ip, sizeof(local_ip));
        printf("server run listen at addr %s, port %hd\n", local_ip, ntohs(local_addr.sin_port));
    }
    signal(SIGCHLD, sig_child);
    pid_t childpid = 0;
    while (1) {
        socklen_t cli_len = 0;
        connfd = accept(svr_fd, (struct sockaddr*)&cli_addr, &cli_len);
        if (connfd < 0) {
            if (errno == EINTR) {
                continue;
            }
            else {
                printf("accept error error no %d", errno);
                return;
            }
        }
        if (childpid = fork() == 0) {
            close(svr_fd);
            // remote ip, port
            struct sockaddr_in remote_addr;
            char remote_ip[20];
            socklen_t remote_len = sizeof(remote_ip);
            //bzero((char*)&remote_addr, sizeof(remote_addr));
            getpeername(connfd, (struct sockaddr*)&remote_addr, &remote_len);
            inet_ntop(AF_INET, &remote_addr.sin_addr, remote_ip, remote_len);
            printf("remote addr %s, port %d\n", remote_ip, ntohs(remote_addr.sin_port));

            char buf[4096];
            memset(buf, 0, 4096);
            size_t readn = 0;
            while ((readn = read(connfd, buf, 4096)) > 0) {
                write(connfd, buf, readn);
            }
            _exit(0);
        }
        close(connfd);
    }
    return 0;
}

void sig_child(int signo) {
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("child %d terminated\n", pid);
    }
}
