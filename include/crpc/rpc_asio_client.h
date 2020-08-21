#pragma once

#include "rpc_client.h"
#include "boost/asio.hpp"

#include "message.h"

struct asio_transport {
    explicit asio_transport(std::string const& name, unsigned int const port)
            : endpoints_(boost::asio::ip::address::from_string(name), port)
    {
        socket_.connect(endpoints_);
    }

    std::vector<unsigned char> send(unsigned fn_idx,
                                    std::vector<unsigned char> const& params) {

        message ms{0, fn_idx, cista::offset::vector<unsigned char>(params.begin(), params.end())};
        write(socket_, boost::asio::buffer(cista::serialize(ms)));

        std::vector<unsigned char> buffer(1024);
        boost::system::error_code error;
        socket_.read_some(boost::asio::buffer(buffer), error);

        if(error) {
            std::cerr << "error while reading\n";
            throw boost::system::system_error(error);
        }
        return buffer;
    }

private:
    boost::asio::io_context io_;
    boost::asio::ip::tcp::endpoint endpoints_;

    boost::asio::ip::tcp::socket socket_{io_};
};

template<typename Interface>
using rpc_asio_client = rpc_client<asio_transport, Interface>;
