#include "address.h"

namespace Sunny
{
Address::Address(const char* addr, short port)
{
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    if (!addr)
    {
        m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    inet_aton(addr, &m_addr.sin_addr);
}

Address::Address(const std::string& addr, short port)
{
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    if (addr.empty())
    {
        m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    inet_aton(addr.c_str(), &m_addr.sin_addr);
}

struct sockaddr* Address::sockaddr()
{
    return (struct sockaddr*)&m_addr;
}

const struct sockaddr* Address::sockaddr() const
{
    return (const struct sockaddr*)&m_addr;
}

std::string Address::address()
{
    char *s = inet_ntoa(m_addr.sin_addr);
    return std::string(s);
}

short Address::port()
{
    return ntohs(m_addr.sin_port);
}

unsigned long Address::netaddr()
{
    return (unsigned long)m_addr.sin_addr.s_addr;
}
}
