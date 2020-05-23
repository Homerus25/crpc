#include <iostream>
#include "cista.h"
#include "Client.h"


int main(int argc, char* argv[])
{
    crpc::Client client("127.0.0.1", 2000);

    std::cout << client.callNoParameter<0, cista::raw::string>() << "\n";

    std::cout << client.call<1, int>(5, 2) << "\n";
    std::cout << client.call<1, int>(10, 100) << "\n";

    client.callNoReturn<2>(cista::raw::string("hi"));

    client.callNoReturnNoArgs<3>();

    return 0;
}
