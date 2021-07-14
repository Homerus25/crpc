#pragma once

#include <boost/asio/thread_pool.hpp>
#include <future>
#include "message.h"
#include "no_network_server.h"
#include "rpc_async_client.h"
#include "ticket_store.h"

struct no_network_transport {
  explicit no_network_transport(std::function<void(const std::vector<uint8_t>, std::function<void(const std::vector<uint8_t>)>)> recv)
      : recv_{recv} {}

  std::future<std::vector<unsigned char>> send(unsigned fn_idx,
                                               std::vector<unsigned char> const& params) {
    message ms {
        ts_.nextNumber(), fn_idx,
        cista::offset::vector<unsigned char>(params.begin(), params.end())};

    auto future = ts_.emplace(ms.ticket_);

    auto const ms_buf = cista::serialize(ms);
    this->recv_(ms_buf, [&](const std::vector<uint8_t> ms){ this->receive(ms); });

    return future;
  }

  void receive(const std::vector<uint8_t> response) {
    const auto* ms = cista::deserialize<message>(response);
    ts_.setValue(ms->ticket_, ms->payload_);
  }

  auto& get_times() {
    return ts_.get_times();
  }

  private:
    std::function<void(const std::vector<uint8_t>, std::function<void(const std::vector<uint8_t>)>)> recv_;
    ticket_store ts_;
};

template <typename Interface>
using no_network_client = rpc_async_client<no_network_transport, Interface>;