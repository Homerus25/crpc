#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <queue>
#include "rpc_server.h"

#include <thread>

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

  void run(int threads_count) {
    std::vector<std::thread> thread_pool;
    for(int i=0; i<threads_count-1; ++i) {
      thread_pool.emplace_back(std::thread([&]() {
        while (alive_) ioc_.run();
      }));
    }

    while(alive_) { ioc_.run(); }

    for (auto& thread : thread_pool) {
      thread.join();
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
