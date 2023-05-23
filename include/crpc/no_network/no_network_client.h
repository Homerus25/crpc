#pragma once

#include <boost/asio/thread_pool.hpp>
#include <future>
#include "crpc/message.h"
#include "crpc/rpc_async_client.h"
#include "crpc/ticket_store.h"
#include "no_network_server.h"

struct no_network_transport {
  explicit no_network_transport(std::function<void(const std::vector<uint8_t>, std::function<void(const std::vector<uint8_t>)>)> recv)
      : recv_{recv} {

    work_guard_ = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(boost::asio::make_work_guard(ioc_));
    std::thread([this](){ioc_.run();}).detach();
  }

  std::future<std::vector<unsigned char>> send(unsigned fn_idx,
                                               std::vector<unsigned char> const& params) {
    message ms {
        ts_.nextNumber(), fn_idx,
        cista::offset::vector<unsigned char>(params.begin(), params.end())};

    auto future = ts_.emplace(ms.ticket_);

    auto const ms_buf = cista::serialize(ms);
    this->recv_(ms_buf, [this](const std::vector<uint8_t> ms){ this->receive(ms); });

    return future;
  }

  void receive(const std::vector<uint8_t>& response) {
    ioc_.post([response, this](){
      const auto* ms = cista::deserialize<message>(response);
      ts_.setValue(ms->ticket_, ms->payload_);
    });
  }

  void stop() {
    ioc_.stop();
  }

  private:
    boost::asio::io_context ioc_;
    std::function<void(const std::vector<uint8_t>, std::function<void(const std::vector<uint8_t>)>)> recv_;
    ticket_store ts_;
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> work_guard_;
};

template <typename Interface>
using no_network_client = rpc_async_client<no_network_transport, Interface>;