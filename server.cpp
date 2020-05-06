#include <iostream>
#include <boost/asio.hpp>

std::string make_daytime_string()
{
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

int main(int argc, char* argv[])
{
    try {
        boost::asio::io_context io;
        boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 2000));

        for(;;) {
            boost::asio::ip::tcp::socket socket(io);
            acceptor.accept(socket);

            std::string message = make_daytime_string();

            boost::system::error_code error;
            boost::asio::write(socket, boost::asio::buffer(message), error);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
