#include <iostream>
#include "Server.h"

#include "cista.h"


crpc::Server::Server(unsigned int port)
    : acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
}

struct cInt { int a; };

void crpc::Server::run()
{
    for(;;) {
        boost::system::error_code error;
        boost::asio::ip::tcp::socket socket(io);
        acceptor.accept(socket);

        std::vector<char> buffer(128);
        size_t len = socket.read_some(boost::asio::buffer(buffer), error);
        //if(std::string_view(buffer.data()) == "getDayTime") {
        std::cout << "got a message: " << std::string_view(buffer.data())<< "\n";
        //if(std::string_view(buffer.data()) == "1") {
        if(cista::deserialize<cInt>(buffer)->a == 0) {
            std::cout << "passed!\n";
            std::cout << "size: " << funcs.size() << "\n";
            //std::string message = make_daytime_string();
            std::vector<char> fromClient;
            auto message = funcs[0](fromClient);
            boost::asio::write(socket, boost::asio::buffer(message), error);

            /*
            cista::offset::string msg(message);
            std::vector<unsigned char> buffer;
            buffer = cista::serialize(msg);

            boost::asio::write(socket, boost::asio::buffer(buffer), error);
            */
        }
    }
}

cista::offset::string rpcTest()
{
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

