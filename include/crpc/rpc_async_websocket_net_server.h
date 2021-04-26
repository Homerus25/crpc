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
        : host_(std::move(host))
        , port_(std::move(port))
        , web_server_{ios}
    {}

    void run();

private:
    std::string host_, port_;
    net::web_server web_server_;
};

template<typename Interface>
void rpc_async_websocket_net_server<Interface>::run()
{
    web_server_.on_ws_msg([&](net::ws_session_ptr const& session,
                             std::string const& msg, net::ws_msg_type type)
                         {
                             auto const req = cista::deserialize<message>(msg);

                             auto const func_num = req->fn_idx;
                             if (func_num < this->fn_.size()) {
                                 //std::cout << "passed with function number: " << func_num << "!\n";
                                 //std::cout << "size: " << this->fn_.size() << "\n";

                                 auto const pload = this->call(func_num,
                                                               std::vector<unsigned char>(req->payload_.begin(),
                                                                                          req->payload_.end()));
                                 message ms{req->ticket_, func_num,
                                            cista::offset::vector<unsigned char>(pload.begin(), pload.end())};

                                 auto const res_buf = cista::serialize(ms);
                                 auto const lock = session.lock();
                                 if (lock) {
                                     lock->send(std::string{begin(res_buf), end(res_buf)},
                                                net::ws_msg_type::BINARY,
                                                [](boost::system::error_code, size_t) {});
                                 }
                             }
                         });
    web_server_.on_ws_open([](net::ws_session_ptr const& session, bool b){ std::cout << "open\n"; });

    boost::system::error_code ec;
    web_server_.init(host_, port_, ec);
    if (!ec) {
        web_server_.run();
    }
}
