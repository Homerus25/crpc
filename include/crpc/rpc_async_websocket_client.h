#pragma once

#include "rpc_async_client.h"

#include "message.h"
#include "deserialize_helper.h"

#include "../ws_client.h"

#include <ctx/ctx.h>
#include <ctx/operation.h>

struct dummy_data {
    void transition(ctx::transition, ctx::op_id, ctx::op_id) {}
};

struct async_websocket_transport {
    explicit async_websocket_transport(std::string const& name, std::string const& port,
            std::function<void()> func)
            : ticket_num_(0)
    {
        ctx::scheduler<dummy_data> sched;
        net_client_ = std::make_unique<net::ws_client>(sched.runner_.ios(), /*ssl_context,*/ name, port);
        net_client_->on_msg([&](std::string const& s, bool const data) {
            auto ms = cista::deserialize<message>(s);
            tickets_.at(ms->ticket_).set_value(std::vector<unsigned char>(ms->payload_.begin(), ms->payload_.end()));
        });

        net_client_->run([&](boost::system::error_code err) {
            if(!err) {
                sched.enqueue_io(dummy_data{}, [&] () {
                    func();
                    net_client_->stop();
                }, ctx::op_id{});
            }
        });

        sched.run(1);
    }

    std::future<std::vector<unsigned char>> send(unsigned fn_idx,
                                                 std::vector<unsigned char> const& params)
    {
        message ms{ticket_num_++, fn_idx, cista::offset::vector<unsigned char>(params.begin(), params.end())};

        std::promise<std::vector<unsigned char>> promise;
        auto future = promise.get_future();
        tickets_.emplace(ms.ticket_, std::move(promise));

        auto const ms_buf = cista::serialize(ms);
        auto const ms_string = std::string(begin(ms_buf), end(ms_buf));
        net_client_->send(ms_string, true);

        return future;
    }

private:
    std::atomic<uint64_t> ticket_num_;
    cista::raw::hash_map<uint64_t, std::promise<std::vector<unsigned char>>> tickets_;
    std::unique_ptr<net::ws_client> net_client_;
};

template<typename Interface>
using rpc_async_websocket_client = rpc_async_client<async_websocket_transport, Interface>;