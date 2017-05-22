#include <iostream>
#include "address.h"
#include "socket.h"

int main()
{
    Address svr_addr("", 9527);
    Socket sock(Socket::SOCK_TCP_STREAM);
    if (!sock.Invalid())
    {
        return -1;
    }
    sock.Bind(&svr_addr);
    sock.Listen();
    Address local_addr;
    sock.getSockName(&local_addr);
    std::cout << "server addr " << local_addr.address() << " port " << local_addr.port() << std::endl;
    Address cli_addr;
    Socket::Size cli_len;
    for (;;)
    {
        int connfd = sock.Accept(&cli_addr, &cli_len);
        sock.close();
    }
    return 0;
}
