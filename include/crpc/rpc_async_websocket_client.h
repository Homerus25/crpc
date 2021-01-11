#pragma once

#include "rpc_async_client.h"

#include "message.h"
#include "deserialize_helper.h"

#include "../ws_client.h"

#include <ctx/ctx.h>
#include <ctx/operation.h>

#include <chrono>

struct dummy_data {
    void transition(ctx::transition, ctx::op_id, ctx::op_id) {}
};

struct async_websocket_transport {
    explicit async_websocket_transport(std::string const& name, std::string const& port,
            std::function<void()> func)
            : ticket_num_(0)
    {
        //ctx::scheduler<dummy_data> sched;
        net_client_ = std::make_unique<net::ws_client>(sched.runner_.ios(), /*ssl_context,*/ name, port);
        net_client_->on_msg([&](std::string const& s, bool const data) {
            auto ms = cista::deserialize<message>(s);
            //std::cout << "nextms\n";
            while(1) {
              auto ticket_iter = tickets_.find(ms->ticket_);
              if(ticket_iter != tickets_.end()) {
                ticket_iter->second.set_value(std::vector<unsigned char>(
                    ms->payload_.begin(), ms->payload_.end()));
                tickets_.erase(ms->ticket_);
                break;
              }
              //std::cout << "failed on rec: " << ms->ticket_ << "\n";
              std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            /*
            try {
              tickets_.at(ms->ticket_)
                  .set_value(std::vector<unsigned char>(ms->payload_.begin(),
                                                        ms->payload_.end()));
              tickets_.erase(ms->ticket_);
              //tickets_.erase_meta_only(ms->ticket_);
            }
            catch (std::exception ex) {
                std::cout << "on rec: " << ex.what() << " ticket num: " << ms->ticket_ << "\n";
            }
            */
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

        /*
        if(tickets_.capacity() < ticket_num_)
          std::cout << "err\n";

        while(tickets_.size() >= tickets_.capacity()) {
          std::cout << "err at size: " << tickets_.size() << "with cap " << tickets_.capacity() << "\n";
          //std::this_thread::sleep_for(std::chrono::duration<std::chrono::milliseconds>(20));
          std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        */

        std::promise<std::vector<unsigned char>> promise;
        auto future = promise.get_future();
        while(!tickets_.emplace(ms.ticket_, std::move(promise)).second) {
          std::cout << "full at size " << tickets_.size() << "\n";
          std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        auto const ms_buf = cista::serialize(ms);
        auto const ms_string = std::string(begin(ms_buf), end(ms_buf));
        net_client_->send(ms_string, true);

        return future;
    }

private:
    std::atomic<uint64_t> ticket_num_;
    cista::raw::hash_map<uint64_t, std::promise<std::vector<unsigned char>>> tickets_;
    std::unique_ptr<net::ws_client> net_client_;
    ctx::scheduler<dummy_data> sched;
};

template<typename Interface>
using rpc_async_websocket_client = rpc_async_client<async_websocket_transport, Interface>;