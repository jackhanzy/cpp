#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/syscall.h>

#define MAXEPOLLSIZE 1000
#define MAXLINE 65535

typedef struct sockaddr* SA;

size_t readn(int fd, void* vptr, size_t n);
size_t writen(int fd, void* vptr, size_t n);

void *thread_func(void *arg);
int setfl(int fd, int flag);

void on_accept(int epoll_fd, int fd);
struct thread_data
{
    int sock_fd;
    int epoll_fd;
};

int main()
{
    int port = 6888;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        fprintf(stderr, "socket failed\n");
        return -1;
    }
    struct sockaddr_in serv_addr, cli_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenfd, (SA)&serv_addr, sizeof(serv_addr)) != 0)
    {
        fprintf(stderr, "bind failed\n");
        return -1;
    }
    listen(listenfd, 1024);
    if (setfl(listenfd, O_NONBLOCK) == -1)
    {
        fprintf(stderr, "%s:%d set socket nonblock failed\n", __FILE__, __LINE__);
        return -1;
    }
    struct epoll_event event;
    int epoll_fd = epoll_create(MAXEPOLLSIZE);
    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listenfd, &event) < 0)
    {
        fprintf(stderr, "epoll ctl failed\n");
        return -1;
    }
    fprintf(stdout, "epoll server listen at port %d\n", port);
    struct thread_data data;
    data.sock_fd = listenfd;
    data.epoll_fd = epoll_fd;
    int i = 0;
    /*pthread_t threads[5];
    for (; i < 5; ++i)
    {
        pthread_create(&threads[i], NULL, thread_func, &data);
    }
    while (1);
    for (i = 0; i < 5; ++i)
    {
        pthread_join(threads[i], NULL);
    }*/
    thread_func(&data);
    close(listenfd);
    return 0;
}

void *thread_func(void *arg)
{
    printf("thread id %d\n", syscall(SYS_gettid));
    //pthread_t pid = pthread_self();
    //printf("thread id %p\n", &pid);
    struct thread_data* pdata = (struct thread_data*)arg;
    int listenfd = pdata->sock_fd;
    int epoll_fd = pdata->epoll_fd;
    char recvbuf[MAXLINE];
    memset(recvbuf, 0, MAXLINE);
    struct epoll_event event;
    struct epoll_event events[MAXEPOLLSIZE];
    struct sockaddr_in cli_addr;
    for (;;)
    {
        int fds = epoll_wait(epoll_fd, events, MAXEPOLLSIZE, -1);
        if (errno == EINTR)
        {
            
        }
        int n = 0;
        for (; n < fds; ++n)
        {
            if (events[n].events & EPOLLERR ||
                events[n].events & EPOLLHUP)
            {
                close(events[n].data.fd);
                fprintf(stderr, "events error\n");
                continue;
            }
            else if (events[n].data.fd == listenfd)
            {
                struct sockaddr_in cli_addr;
                int cli_len = sizeof(cli_addr);
                bzero(&cli_addr, sizeof(cli_addr));
                int connfd = 0;
                if ((connfd = accept(listenfd, (SA)&cli_addr, &cli_len) > 0))
                {
                    setfl(connfd, O_NONBLOCK);
                    event.events = EPOLLIN | EPOLLET;
                    event.data.fd = connfd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &event);
                    //on_accept(epoll_fd, connfd);
                    char remote_ip[20];
                    inet_ntop(AF_INET, &cli_addr.sin_addr, remote_ip, 20);
                    fprintf(stdout, "accept new connection fd %d, rempte ip %s, port %hd\n", connfd, remote_ip, ntohs(cli_addr.sin_port));
                }
            }
            else if (events[n].events & EPOLLIN)
            {
                int fd = events[n].data.fd;
                size_t n = read(fd, recvbuf, MAXLINE - 1);
                //while ((n = read(fd, recvbuf, MAXLINE - 1)) > 0);
                fprintf(stdout, "receive client data, connfd %d data size %d\n", fd, n);
                if (n == 0)
                {
                    event.data.fd = fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event);
                    close(fd);
                    fprintf(stdout, "remote close connection, connfd %d\n", fd);
                }
                event.events = EPOLLOUT | EPOLLET;
                event.data.fd = fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
            }
            else if (events[n].events & EPOLLOUT)
            {
                int fd = events[n].data.fd;
                write(fd, recvbuf, strlen(recvbuf));
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
                fprintf(stdout, "send client data, connfd %d\n", fd);
            }
        }
    }
    return (void*)0;
}

int setfl(int fd, int flag)
{
    int val = fcntl(fd, F_GETFL, 0);
    if (val < 0)
    {
        return -1;
    }
    if (fcntl(fd, F_SETFL, val | flag) < 0)
    {
        return -1;
    }
    return 0;
}

void on_accept(int epoll_fd, int connfd)
{
    struct epoll_event event;
    setfl(connfd, O_NONBLOCK);
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = connfd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &event);
}

size_t readn(int fd, void* vptr, size_t n)
{
    size_t nread = 0;
    size_t nleft = n;
    char *ptr = vptr;
    while (nleft > 0)
    {
        if ((nread = read(fd, ptr, nleft)) < 0)
        {
            if (errno == EINTR)
            {
                nread = 0;
            }
            else
            {
                return -1;
            }
        }
        else if (nread == 0)
        {
            break;
        }
        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft);
}
size_t writen(int fd, void* vptr, size_t n)
{
    size_t nwrite = 0;
    size_t nleft = n;
    char *ptr = vptr;
    while (nleft > 0)
    {
        nwrite = write(fd, ptr, n);
        if (nwrite <= 0 && errno == EINTR)
        {
            nwrite = 0;
        }
        else
        {
            return -1;
        }
        nleft -= nwrite;
        ptr += nwrite;
    }
    return n;
}
