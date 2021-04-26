#pragma once

#include "rpc_server.h"
#include "message.h"
#include "deserialize_helper.h"
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <thread>

#include <iostream>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

template <typename Interface>
class rpc_async_thread_websocket_server : public rpc_server<Interface> {
public:
    explicit rpc_async_thread_websocket_server(unsigned int port);

    [[noreturn]] void run();

private:
    net::io_context io_;
    boost::asio::ip::tcp::acceptor acceptor_;

    void listen_to_client(tcp::socket);

};

template <typename Interface>
rpc_async_thread_websocket_server<Interface>::rpc_async_thread_websocket_server(unsigned int port)
        : io_{1}
        , acceptor_(io_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                                                        port))
{
}

template<typename Interface>
[[noreturn]] void rpc_async_thread_websocket_server<Interface>::run()
{
    for(;;)
    {
        tcp::socket socket{io_};
        acceptor_.accept(socket);

        std::thread t(std::mem_fn(&rpc_async_thread_websocket_server<Interface>::listen_to_client), this, std::move(socket));
        t.detach();
    }
}

template<typename Interface>
void rpc_async_thread_websocket_server<Interface>::listen_to_client(tcp::socket socket)
{
    try {
      websocket::stream<tcp::socket> ws{std::move(socket)};
      ws.accept();
      ws.binary(true);

      for(;;)
      {
          if(!ws.is_open())
              return;
          beast::flat_buffer buffer;
          ws.read(buffer);

          Container con(static_cast<unsigned char*>(buffer.data().data()), buffer.size());
          auto c = cista::deserialize<message>(con);

          auto const func_num = c->fn_idx;
          if (func_num < this->fn_.size()) {
              auto const pload = this->call(func_num, std::vector<unsigned char>(c->payload_.begin(), c->payload_.end()));
              message ms{ c->ticket_, func_num, cista::offset::vector<unsigned char>(pload.begin(), pload.end()) };
              ws.write(boost::asio::buffer(cista::serialize(ms)));
          }
      }
    }
    catch(beast::system_error const& se)
    {
        // This indicates that the session was closed
        if(se.code() != websocket::error::closed)
            std::cerr << "Error: " << se.code().message() << std::endl;
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

