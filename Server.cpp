#include "Server.h"

#include "cista.h"
#include <iostream>


crpc::Server::Server(unsigned int port)
    : acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    , buffer(1024)
{
}

struct cInt { int a; };

void crpc::Server::run()
{
    for(;;) {
        boost::system::error_code error;
        boost::asio::ip::tcp::socket socket(io);
        acceptor.accept(socket);

        size_t len = socket.read_some(boost::asio::buffer(buffer), error);

        std::cout << "got a message: " << std::string_view(buffer.data())<< "\n";

        params.clear();
        params.resize(buffer.end() - (buffer.begin() + sizeof(cInt)));
        std::copy(buffer.begin() + sizeof(cInt), buffer.end(), params.begin());

        int funcNum = cista::deserialize<cInt>(buffer)->a;
        if(funcNum < funcs.size()) {
            std::cout << "passed with function number: " << funcNum << "!\n";
            std::cout << "size: " << funcs.size() << "\n";

            auto message = funcs[funcNum]();
            boost::asio::write(socket, boost::asio::buffer(message), error);
        }
    }
}


void crpc::Server::test()
{
    struct testStr {int a; int b; };
    testStr st{5, 2 };
    params = cista::serialize(st);

    auto message = funcs[1]();
    std::cout << *cista::deserialize<int>(message);
}
