#include <iostream>
#include "cista.h"
#include "Client.h"


int main(int argc, char* argv[])
{
    crpc::Client client("127.0.0.1", 2000);
    std::cout << client.call<0, cista::raw::string>();

    std::cout << client.call<1, int>(5, 2);

    return 0;
}
