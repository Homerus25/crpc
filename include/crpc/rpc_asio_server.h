#pragma once

#include "rpc_server.h"
#include "message.h"
#include "boost/asio.hpp"

#include <iostream>

template <typename Interface>
class rpc_asio_server : public rpc_server<Interface> {
public:
    explicit rpc_asio_server(unsigned int port);

    void run();

private:
    void listen_to_client(boost::asio::ip::tcp::socket&);

    boost::system::error_code receive(boost::asio::ip::tcp::socket& socket, std::vector<char>&);
    boost::system::error_code send(boost::asio::ip::tcp::socket& socket, const std::vector<unsigned char>&);

    boost::asio::io_context io_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

template<typename Interface>
void rpc_asio_server<Interface>::run()
{
    while (true) {
        boost::asio::ip::tcp::socket socket(io_);
        acceptor_.accept(socket);
        listen_to_client(socket);
    }
}

template<typename Interface>
void rpc_asio_server<Interface>::listen_to_client(boost::asio::ip::tcp::socket& socket)
{
    std::vector<char> buffer_(1024);
    boost::system::error_code error;
    for (;;) {
        buffer_.assign(1024, 0);
        if (receive(socket, buffer_)) {
            std::cout << "error while reading!\n";
            throw boost::system::system_error(error);
            return;
        }

       // std::cout << "got a message: " << std::string_view(buffer_.data()) << "\n";

        auto const c = cista::deserialize<message>(buffer_);
        auto const func_num = c->fn_idx;
        if (func_num < this->fn_.size()) {
            std::cout << "passed with function number: " << func_num << "!\n";
            std::cout << "size: " << this->fn_.size() << "\n";

            auto const pload = this->call(func_num, std::vector<unsigned char>(c->payload_.begin(), c->payload_.end()));
            message ms{ c->ticket_, func_num, cista::offset::vector<unsigned char>(pload.begin(), pload.end()) };
            error = send(socket, cista::serialize(ms));
        }

        if (error) {
            throw boost::system::system_error(error);
        }
    }
}

template<typename Interface>
boost::system::error_code rpc_asio_server<Interface>::receive(boost::asio::ip::tcp::socket& socket, std::vector<char>& buffer)
{
    boost::system::error_code error;
    auto size = socket.read_some(boost::asio::buffer(buffer), error);

    if(error)
        return error;

    const auto bufferSize = buffer.size();

    while(size == bufferSize){
        std::vector<char> buffer2(bufferSize);
        size = socket.read_some(boost::asio::buffer(buffer), error);

        if(error)
            return error;

        buffer.insert(buffer.end(), buffer2.begin(), buffer2.begin() + size);
    }

    return error;
}

template<typename Interface>
boost::system::error_code rpc_asio_server<Interface>::send(boost::asio::ip::tcp::socket& socket, const std::vector<unsigned char>& message)
{
    boost::system::error_code error;
    boost::asio::write(socket, boost::asio::buffer(message), error);
    return error;
}

template<typename Interface>
rpc_asio_server<Interface>::rpc_asio_server(unsigned int port)
    : acceptor_(io_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                                                        port))
{
}
