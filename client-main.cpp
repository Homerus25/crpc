#include <iostream>
#include "cista.h"
#include "Client.h"

struct CustomClient : public crpc::Client {
    explicit CustomClient(const std::string& name, unsigned int port)
        : Client(name, port)
        , make_daytime_string(this)
        , addTwoNums(this)
        , print(this)
        , printNoArgs(this)
    {}

    crpc::Client::fn<0, cista::raw::string> make_daytime_string;
    crpc::Client::fn<1, int, int, int> addTwoNums;
    crpc::Client::fn<2, void, cista::raw::string> print;
    crpc::Client::fn<3, void> printNoArgs;
};


int main(int argc, char* argv[])
{
    CustomClient client("127.0.0.1", 2000);

    std::cout << client.make_daytime_string();

    std::cout << client.addTwoNums(5, 2) << "\n";
    std::cout << client.addTwoNums(10, 100) << "\n";

    client.print("hi");

    client.printNoArgs();

    return 0;
}
