#pragma once

#include "rpc_async_client.h"

#include "message.h"
#include "deserialize_helper.h"
#include "boost/beast/core.hpp"
#include <boost/beast/websocket.hpp>
#include "boost/asio.hpp"

#include "ticket_store.h"

#include <cstdlib>
#include <thread>
#include <functional>
#include <boost/asio/spawn.hpp>

namespace beast = boost::beast;
//namespace net = boost::asio;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;

struct async_thread_websocket_transport {
    explicit async_thread_websocket_transport(std::string const& name, unsigned int const port)
            : endpoints_(boost::asio::ip::address::from_string(name), port)
            , ws(boost::asio::make_strand(io_))
    {
        ws.next_layer().connect(endpoints_);

        std::string host = name + ':' + std::to_string(port);
        ws.handshake(host, "/");

        ws.binary(true);

        std::thread( [&] () {
          boost::asio::spawn(
              io_,
              std::bind(boost::beast::bind_front_handler(
                            &async_thread_websocket_transport::receive, this),
                        std::placeholders::_1));
        });

        io_.run();
    }

    std::future<std::vector<unsigned char>> send(unsigned fn_idx,
                                                 std::vector<unsigned char> const& params)
    {
        message ms {
            ts_.nextNumber(), fn_idx,
            cista::offset::vector<unsigned char>(params.begin(), params.end())};
        auto future = ts_.emplace(ms.ticket_);

        ws.write(boost::asio::buffer(cista::serialize(ms)));
        return future;
    }

  [[noreturn]] void receive(boost::asio::yield_context y) {
    while(true) {
      beast::flat_buffer buffer;
      buffer.reserve(1024);

      boost::system::error_code error_code;
      ws.async_read(buffer, y[error_code]);
      if (error_code != boost::system::errc::errc_t::success) {
        std::cerr << "read nothing! error: " << error_code.message()
                  << std::endl;
        continue;
      }

      Container con(static_cast<unsigned char*>(buffer.data().data()), buffer.size());
      auto ms = cista::deserialize<message>(con);
      ts_.setValue(ms->ticket_, ms->payload_);
    }
  }

  ~async_thread_websocket_transport() {
    boost::system::error_code ec;
    ws.async_close(websocket::close_code::normal, [](boost::system::error_code ec){});
  }

private:
    boost::asio::io_context io_;
    boost::asio::ip::tcp::endpoint endpoints_;
    websocket::stream<tcp::socket> ws;
    ticket_store ts_;
};

template<typename Interface>
using rpc_async_thread_websocket_client = rpc_async_client<async_thread_websocket_transport, Interface>;