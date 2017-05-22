#ifndef __SUNNY_SOCKET_H__
#define __SUNNY_SOCKET_H__

#include <string>
namespace Sunny
{
class Address;
class Socket
{
public:
    typedef socklen_t Size;
    static const size_t MAX_LENGTH = 4096;
    enum
    {
        SOCK_TCP_STREAM = 1,
        AOCK_UDP_STREAM = 2,
    };
public:
    Socket(int type);
    int sockfd() { return m_sockfd; }
    bool Invalid() { return m_sockfd > 0; }

    int Connect(Address* addr);
    int Bind(Address* addr);
    int Listen();
    int Accept(Address* cli_addr, Size* addrlen);
    int Close();
    int Shutdown();
    int GetSockName(Address* local_addr, Size* len);
    int GetPeerName(Address* peer_addr, Size* len);

    int Read(std::string& data);
    int Read(char* data, size_t n);

    int Write(const std::string& data);
    int Write(const char* data, size_t n);
private:
    int m_sockfd;
};

}

#endif
