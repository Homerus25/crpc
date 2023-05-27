#pragma once

#include <boost/asio/thread_pool.hpp>
#include <future>
#include "crpc/message.h"
#include "crpc/rpc_async_client.h"
#include "crpc/ticket_store.h"
#include "no_network_server.h"

struct no_network_transport {
  explicit no_network_transport(Receiver rec, std::function<void(const std::vector<uint8_t>, std::function<void(const std::vector<uint8_t>)>)> recv)
      : receiver(rec), recv_{recv} {

    work_guard_ = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(boost::asio::make_work_guard(ioc_));
    runner = std::thread([&](){ ioc_.run();});
  }

  void send(std::vector<unsigned char> ms_buf) {
    this->recv_(ms_buf, [this](const std::vector<uint8_t> ms){ this->receive(ms); });
  }

  void receive(const std::vector<uint8_t>& response) {
    std::shared_ptr<std::vector<uint8_t>> ptr = std::make_shared<std::vector<uint8_t>>(std::move(response));
    ioc_.post([this, ptr](){
      receiver.processAnswer(*ptr);
    });
  }

  void stop() {
    work_guard_.reset();
    runner.join();
  }

  private:
    Receiver receiver;
    boost::asio::io_context ioc_;
    std::function<void(const std::vector<uint8_t>, std::function<void(const std::vector<uint8_t>)>)> recv_;
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> work_guard_;
    std::thread runner;
};

template <typename Interface>
using no_network_client = rpc_async_client<no_network_transport, Interface>;