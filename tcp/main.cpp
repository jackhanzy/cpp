#include <iostream>
#include "address.h"
using namespace std;

int main()
{
    Sunny::Address addr("127.0.0.1", 8080);
    cout << addr.port() << endl;
    cout << addr.address() << endl;
    cout << addr.net_addr() << endl;
    return 0;
}
