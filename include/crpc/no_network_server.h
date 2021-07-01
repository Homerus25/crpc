#pragma once

#include <boost/lockfree/stack.hpp>
#include "rpc_server.h"

template <typename Interface>
struct no_network_server : public rpc_server<Interface> {
  explicit no_network_server() : queue_(1024), alive_(true) {}

  void receive(const std::vector<uint8_t> message, std::function<void(const std::vector<uint8_t>)> client) {
    queue_.push(std::make_pair(message, client));
  }

  void run() {
    while(alive_) {
        queue_.consume_all_atomic(
            [&](std::pair<std::vector<uint8_t>,
                          std::function<void(std::vector<uint8_t>)>>
                    ms) {
              auto const response = this->process_message(ms.first);
              if (response)
                ms.second(response.value());
            });
    }
  }

  void kill() {
    alive_.store(false);
  }

private:

  boost::lockfree::stack<std::pair<std::vector<uint8_t>, std::function<void(const std::vector<uint8_t>)>>> queue_;
  std::atomic_bool alive_;
};
