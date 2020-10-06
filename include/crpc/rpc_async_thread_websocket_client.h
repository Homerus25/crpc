#pragma once

#include "rpc_async_client.h"

#include "message.h"
#include "deserialize_helper.h"
#include <boost/beast/websocket.hpp>
#include "boost/asio.hpp"

namespace beast = boost::beast;
//namespace net = boost::asio;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;

struct async_thread_websocket_transport {
    explicit async_thread_websocket_transport(std::string const& name, unsigned int const port)
            : endpoints_(boost::asio::ip::address::from_string(name), port)
            , ticket_num_(0)
    {
        ws.next_layer().connect(endpoints_);

        std::string host = name + ':' + std::to_string(port);
        ws.handshake(host, "/");

        ws.binary(true);

        std::thread t(&async_thread_websocket_transport::receive, this);
        t.detach();
    }

    std::future<std::vector<unsigned char>> send(unsigned fn_idx,
                                                 std::vector<unsigned char> const& params)
    {
        message ms{ticket_num_++, fn_idx, cista::offset::vector<unsigned char>(params.begin(), params.end())};

        std::promise<std::vector<unsigned char>> promise;
        auto future = promise.get_future();
        tickets_.emplace(ms.ticket_, std::move(promise));

        ws.write(boost::asio::buffer(cista::serialize(ms)));

        return future;
    }

    void receive() {
        while(1) {
            beast::flat_buffer buffer;
            ws.read(buffer);

            Container con(static_cast<unsigned char*>(buffer.data().data()), buffer.size());
            auto ms = cista::deserialize<message>(con);
            tickets_.at(ms->ticket_).set_value(std::vector<unsigned char>(ms->payload_.begin(), ms->payload_.end()));
        }
    }

private:
    boost::asio::io_context io_;
    boost::asio::ip::tcp::endpoint endpoints_;

    websocket::stream<tcp::socket> ws{io_};

    std::atomic<uint64_t> ticket_num_;
    cista::raw::hash_map<uint64_t, std::promise<std::vector<unsigned char>>> tickets_;
};

template<typename Interface>
using rpc_async_thread_websocket_client = rpc_async_client<async_thread_websocket_transport, Interface>;