#pragma once

#include "../rpc_async_client.h"
#include "../message.h"
#include "../log.h"
#include "../ticket_store.h"

#include "../../ws_client.h"
#include "boost/asio/post.hpp"

#include <memory>

struct ws_transport {
  explicit ws_transport(boost::asio::io_context& io, std::string const& url, unsigned int const port) :ioc_(io)
  {
    std::promise<void> pr;
    client = std::make_unique<net::ws_client>(ioc_, url, std::to_string(port));
    client->run([&](boost::system::error_code ec) {
      if (ec) {
        std::cout << "connect failed: " << ec.message() << "\n";
        return;
      }
      std::cout << "connected\n";
      pr.set_value();
    });
    client->on_fail([&](boost::system::error_code ec) {
      std::cout << "fail " << ec.to_string() << std::endl;
    });

    client->on_msg([&](std::string const& msg, bool /* binary */) {
      data::vector<unsigned char> dd(msg.begin(), msg.end());
      auto ms = cista::deserialize<message>(dd);
      this->ts_.setValue(ms->ticket_, ms->payload_);
    });

    auto fut = pr.get_future();

    while (true) {
      std::future_status status = fut.wait_for(std::chrono::seconds(1));
      if (status == std::future_status::ready) break;
      ioc_.poll();
    }
    std::cout << "is connected" << std::endl;
  }

  std::future<std::vector<unsigned char>> send(unsigned fn_idx,
                                  std::vector<unsigned char> const& params) {
    std::string sp(params.begin(), params.end());
    std::cout << "send params: " << sp << std::endl;
    message ms{
        ts_.nextNumber(), fn_idx,
        cista::offset::vector<unsigned char>(params.begin(), params.end())};
    auto future = ts_.emplace(ms.ticket_);

    auto const ms_buf = cista::serialize(ms);
    auto const ms_string = std::string(begin(ms_buf), end(ms_buf));
    std::cout << "send" << ms_string << std::endl;
    boost::asio::post(ioc_,
        [=, this]() {
          client->send(ms_string, true);
        }
        );

    return future;
  }

private:
  boost::asio::io_context& ioc_;
  std::unique_ptr<net::ws_client> client;
  ticket_store ts_;
};

template<typename Interface>
using rpc_ws_client = rpc_async_client<ws_transport, Interface>;
