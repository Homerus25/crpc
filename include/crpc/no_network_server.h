#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <queue>
#include "rpc_server.h"

#include <iostream>

template <typename Interface>
struct no_network_server : public rpc_server<Interface> {
  explicit no_network_server() : ioc_(1), alive_(true) {}

  void receive(const std::vector<uint8_t> message, std::function<void(const std::vector<uint8_t>)> client) {
    boost::asio::post(
        [cl = client, ms = message, this](){
      auto const response = this->process_message(ms);
      if(response)
        cl(std::move(response.value()));
    });
  }

  void run() {
    while(alive_) {
      ioc_.run();
    }
  }

  void kill() {
    alive_.store(false);
    ioc_.stop();
  }

private:
  boost::asio::io_context ioc_;
  std::atomic_bool alive_;
};
