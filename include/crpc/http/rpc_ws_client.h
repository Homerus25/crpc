#pragma once

#include "../rpc_async_client.h"
#include "../message.h"
#include "../log.h"
#include "../ticket_store.h"

#include "../../ws_client.h"
#include "boost/asio/post.hpp"

#include <memory>

struct ws_transport {
  explicit ws_transport(std::string const& url, unsigned int const port)
  {
    client = std::make_unique<net::ws_client>(ios, url, std::to_string(port));
    client->run([&](boost::system::error_code ec) {
      if (ec) {
        std::cout << "connect failed: " << ec.message() << "\n";
        return;
      }
      std::cout << "connected\n";
    });
    client->on_fail([&](boost::system::error_code ec) {
      std::cout << "fail " << ec.to_string() << std::endl;
    });

    client->on_msg([&](std::string const& msg, bool /* binary */) {
      std::vector<unsigned char> dd(msg.begin(), msg.end());
      auto ms = cista::deserialize<message>(dd);
      this->ts_.setValue(ms->ticket_, ms->payload_);
    });



    std::thread t1([&](){ ios.run(); });
    t1.detach();
  }

  std::future<std::vector<unsigned char>> send(unsigned fn_idx,
                                  std::vector<unsigned char> const& params) {
    message ms{
        ts_.nextNumber(), fn_idx,
        cista::offset::vector<unsigned char>(params.begin(), params.end())};
    auto future = ts_.emplace(ms.ticket_);

    auto const ms_buf = cista::serialize(ms);
    auto const ms_string = std::string(begin(ms_buf), end(ms_buf));
    std::cout << "send" << ms_string << std::endl;
    boost::asio::post([=]() {
      client->send(ms_string, true);
    });

    return future;
  }

private:
  boost::asio::io_service ios;
  std::unique_ptr<net::ws_client> client;
  ticket_store ts_;
};

template<typename Interface>
using rpc_ws_client = rpc_async_client<ws_transport, Interface>;
