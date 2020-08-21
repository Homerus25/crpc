#include "server.hpp"

#include "../include/crpc/message.h"

#include <iostream>

namespace project
{
    template <typename interface>
    server<interface>::server(net::executor exec)
    : acceptor_(exec)
    {
        auto ep = net::ip::tcp::endpoint(net::ip::address_v4::any(), 4321);
        acceptor_.open(ep.protocol());
        acceptor_.set_option(net::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(ep);
        acceptor_.listen();
    }

    template <typename interface>
    void server<interface>::run()
    {
        net::co_spawn(
            acceptor_.get_executor(),
            [this]() -> net::awaitable< void > { co_await this->handle_run(); },
            net::detached);
    }

    template <typename interface>
    void server<interface>::stop()
    {
        net::dispatch(net::bind_executor(acceptor_.get_executor(), [this] { this->handle_stop(); }));
    }

    template <typename interface>
    net::awaitable< void > server<interface>::handle_run()
    {
        while (!ec_)
            try
            {
                auto sock = co_await acceptor_.async_accept(net::use_awaitable);
                auto ep   = sock.remote_endpoint();
                sock.binary(true);
                auto conn = std::make_shared< connection_impl >(std::move(sock));
                // cache the connection
                connections_[ep] = conn;
                conn->run(
                        [&] (std::string incomingMessage)
                        {
                            auto c = cista::deserialize<message>(incomingMessage);

                            auto const func_num = c->fn_idx;
                            if (func_num < this->fn_.size()) {
                                std::cout << "passed with function number: " << func_num << "!\n";
                                std::cout << "size: " << this->fn_.size() << "\n";

                                auto const pload = this->call(func_num, std::vector<unsigned char>(c->payload_.begin(), c->payload_.end()));
                                message ms{ c->ticket_, func_num, cista::offset::vector<unsigned char>(pload.begin(), pload.end()) };
                                conn->send(cista::serialize(ms));
                            }
                        }
                        );
                /*
                conn->send("Welcome to my websocket server!\n");
                conn->send("You are visitor number " + std::to_string(connections_.size()) + "\n");
                conn->send("You connected from " + ep.address().to_string() + ":" + std::to_string(ep.port()) + "\n");
                conn->send("Be good!\n");
                 */
            }
            catch (system_error &se)
            {
                if (se.code() != net::error::connection_aborted)
                    throw;
            }
    }

    template <typename interface>
    void server<interface>::handle_stop()
    {
        ec_ = net::error::operation_aborted;
        acceptor_.cancel();
        for (auto &[ep, weak_conn] : connections_)
            if (auto conn = weak_conn.lock())
            {
                std::cout << "stopping connection on " << ep << std::endl;
                conn->stop();
            }
        connections_.clear();
    }
}   // namespace project