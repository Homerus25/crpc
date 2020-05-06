#include <iostream>
#include <boost/asio.hpp>

int main(int argc, char* argv[])
{
    std::string host = "127.0.0.1";
    try
    {
        boost::asio::io_context io;
        boost::asio::ip::tcp::endpoint endpoints(boost::asio::ip::address::from_string(host), 2000);

        boost::asio::ip::tcp::socket socket(io);
        socket.connect(endpoints);

        for(;;) {
            std::vector<char> buffer(128);
            boost::system::error_code error;
            size_t len = socket.read_some(boost::asio::buffer(buffer), error);

            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            std::cout.write(buffer.data(), len);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
