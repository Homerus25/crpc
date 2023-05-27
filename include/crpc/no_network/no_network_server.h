#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <queue>
#include "crpc/rpc_server.h"

#include <thread>

template <typename Interface>
struct no_network_server : public rpc_server<Interface> {
  explicit no_network_server()
    : work_guard_(boost::asio::make_work_guard(ioc_))
  {}

  void receive(const std::vector<uint8_t> message, std::function<void(const std::vector<uint8_t>)> client) {
    ioc_.post(
        [cl = client, ms = std::move(message), this](){
      auto const response = this->process_message(std::move(ms));
      if(response)
        cl(std::move(response.value()));
    });
  }

  void run(int threads_count) {
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> x
        = boost::asio::make_work_guard(ioc_);

    for(int i=0; i<threads_count; ++i) {
      runner.emplace_back([&](){ioc_.run();});
    }
  }

  void stop() {
    work_guard_.reset();

    for(auto& t: runner)
      t.join();

    ioc_.stop();
  }

private:
  boost::asio::io_context ioc_;
  std::vector<std::thread> runner;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
};
