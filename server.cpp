#include "server.h"

#include "cista.h"

std::string make_daytime_string()
{
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

crpc::server::server(unsigned int port)
    : acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
}

void crpc::server::run()
{
    for(;;) {
        boost::asio::ip::tcp::socket socket(io);
        acceptor.accept(socket);

        std::string message = make_daytime_string();
        cista::offset::string msg(message);
        std::vector<unsigned char> buffer;
        buffer = cista::serialize(msg);

        boost::system::error_code error;
        boost::asio::write(socket, boost::asio::buffer(buffer), error);
    }
}
