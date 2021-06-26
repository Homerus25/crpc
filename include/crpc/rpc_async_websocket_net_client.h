#pragma once

#include "rpc_async_client.h"

#include "message.h"
#include "deserialize_helper.h"

#include "../ws_client.h"

#include <ctx/ctx.h>
#include <ctx/operation.h>

#include <chrono>

#include "ticket_store.h"

struct dummy_data {
    void transition(ctx::transition, ctx::op_id, ctx::op_id) {}
};

struct async_websocket_net_transport {
    explicit async_websocket_net_transport(std::string const& name, std::string const& port,
            std::function<void()> func)
    {
        net_client_ = std::make_unique<net::ws_client>(sched.runner_.ios(), /*ssl_context,*/ name, port);
        net_client_->on_msg([&](std::string const& s, bool const data) {
            auto ms = cista::deserialize<message>(s);
            ts_.setValue(ms->ticket_, ms->payload_);
        });

      //net_client_->on_fail([](boost::system::error_code err) { std::cerr << "failed: " << err.message() << std::endl; });

        net_client_->run([&](boost::system::error_code err) {
            if(!err) {
                sched.enqueue_io(dummy_data{}, [&] () {
                    func();
                    net_client_->stop();
                }, ctx::op_id{});
            }
            else {
              std::cerr << "Can not run! " << err.message() << std::endl;
            }
        });

        sched.run(1);
    }

    std::future<std::vector<unsigned char>> send(unsigned fn_idx,
                                                 std::vector<unsigned char> const& params)
    {
      message ms {
        ts_.nextNumber(), fn_idx,
        cista::offset::vector<unsigned char>(params.begin(), params.end())};
      auto future = ts_.emplace(ms.ticket_);

      auto const ms_buf = cista::serialize(ms);
      auto const ms_string = std::string(begin(ms_buf), end(ms_buf));
      net_client_->send(ms_string, true);

      return future;
    }

private:
    ticket_store ts_;
    std::unique_ptr<net::ws_client> net_client_;
    ctx::scheduler<dummy_data> sched;
};

template<typename Interface>
using rpc_async_websocket_client = rpc_async_client<async_websocket_net_transport, Interface>;