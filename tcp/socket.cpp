#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include "address.h"
#include "socket.h"

namespace Sunny
{
Socket::Socket(int type)
{
    int t;
    if (type == SOCK_TCP_STREAM)
    {
        t = SOCK_STREAM;
    }
    else
    {
        t = SOCK_DGREAM;
    }
    m_sockfd = socket(AF_INET, t, 0);
}

int Socket::Connect(Address* addr)
{
    const struct sockaddr *sockaddr = addr->sockaddr();
    socklen_t addrlen = addr->len();
    return connect(m_sockfd, sockaddr, addrlen);
}
int Socket::Bind(Address* addr)
{
    return bind(m_sockfd, addr->sockaddr(), addr->len());
}

int Socket::Listen()
{
    return listen(m_sockfd, 0);
}

int Socket::Accept(Address* cli_addr, Size* addrlen)
{
    return accept(m_sockfd, cli_addr->sockaddr(), addrlen);
}
int Socket::Close()
{
    close(m_sockfd);
}
int Socket::Shutdown()
{

}
int Socket::GetSockName(Address* local_addr, Size* len)
{
    return getsockname(m_sockfd, local_addr->sockaddr(), (socklen_t*)len);
}
int Socket::GetPeerName(Address* peer_addr, Size* len)
{
    return getpeername(m_sockfd, peer_addr->sockaddr(), (socklen_t*)len);
}

int Socket::Read(std::string& data)
{
    char s[MAX_LENGTH];
    size_t nread = Read(s, MAX_LENGTH);
    if (nread < 0)
    {
        return nread;
    }
    data.assign(s, nread);
    return nread;
}
int Socket::Read(char* data, size_t n)
{
    size_t nleft = n;
    size_t nread;
    char *ptr = data;
    while (nleft > 0)
    {
        if ((nread = read(m_sockfd, ptr, nleft)) < 0)
        {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        }
        else if (nread == 0)
        {
            break;
        }
        nleft -= nread;
        ptr += nread;
    }
    return n - nleft;
}

int Socket::Write(const std::string& data)
{
    return Write(data.c_str(), data.size());
}
int Socket::Write(const char* data, size_t n)
{
    size_t nleft = n;
    size_t nwrite;
    const char *ptr = data;
    while (nleft > 0)
    {
        if (nwrite = write(m_sockfd, ptr, nleft) <= 0)
        {
            if (nwrite < 0 && errno == EINTR)
                nwrite = 0;
            else
                return -1;
        }
        nleft -= nwrite;
        ptr += nwrite;
    }
    return n;
}
}
