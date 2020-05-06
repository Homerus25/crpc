#pragma once

#include <boost/asio.hpp>

namespace crpc
{
    class Client {
    public:
        Client(std::string name, unsigned int port);

        std::string getDayTime();

    private:
        boost::asio::io_context io;
        boost::asio::ip::tcp::endpoint endpoints;//(boost::asio::ip::address::from_string(host), 2000);
    };
}
