#ifndef __IPV4_ADDRESS_H__
#define __IPV4_ADDRESS_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

namespace Sunny
{
class Address
{
public:
    Address() {}
    Address(const char* addr, short port);
    Address(const std::string& addr, short port);

    struct sockaddr* sockaddr();

    const struct sockaddr* sockaddr() const;

    std::string address();
    short port();

    socklen_t len() { return sizeof(m_addr); }
    unsigned long netaddr();

private:
    struct sockaddr_in m_addr;
};
}
#endif
