#pragma once

#include <boost/asio.hpp>

namespace crpc {

    class server {
    public:
        explicit server(unsigned int port);

        void run();

    private:
        boost::asio::io_context io;
        boost::asio::ip::tcp::acceptor acceptor;
    };

}
