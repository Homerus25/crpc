#pragma once

#include "rpc_server.h"
#include "message.h"
#include "deserialize_helper.h"

#include <boost/asio.hpp>

#include "net/web_server/web_server.h"

#include <iostream>

template <typename Interface>
class rpc_async_websocket_net_server : public rpc_server<Interface> {
public:
    explicit rpc_async_websocket_net_server(boost::asio::io_service& ios, std::string host, std::string port)
        : ios_(ios)
        , host_(std::move(host))
        , port_(std::move(port))
        , web_server_{ios}
    {}

    void run(int);

private:
    boost::asio::io_service& ios_;
    std::string host_, port_;
    net::web_server web_server_;
};

template<typename Interface>
void rpc_async_websocket_net_server<Interface>::run(int threads_count)
{
    web_server_.set_request_queue_limit(1024);
    web_server_.on_ws_msg([&](net::ws_session_ptr const& session,
                             std::string const& msg, net::ws_msg_type type)
                         {
                            auto const response = this->template process_message(msg);
                             if(response) {
                               auto const lock = session.lock();
                               if (lock) {
                                 auto res_buf = response.value();
                                 lock->send(std::string{begin(res_buf), end(res_buf)},
                                            net::ws_msg_type::BINARY,
                                            [](boost::system::error_code, size_t) {});
                               }
                             }
                         });
    web_server_.on_ws_open([&](net::ws_session_ptr const& session, bool b){});
    //web_server_.on_ws_close([](void*){ std::cout << "close\n"; });

    boost::system::error_code ec;
    web_server_.init(host_, port_, ec);
    if (!ec) {
        web_server_.run();
    }

  std::vector<std::thread> thread_pool;
  for(int i=0; i<threads_count-1; ++i)
    thread_pool.emplace_back(std::thread([&](){ ios_.run(); }));

  ios_.run();
  for (auto& thread : thread_pool)
    thread.join();
}
