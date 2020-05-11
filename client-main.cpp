#include <iostream>
#include "cista.h"
#include "Client.h"


int main(int argc, char* argv[])
{
    crpc::Client client("127.0.0.1", 2000);
    std::cout << client.call<0, cista::raw::string>();

    return 0;
}
