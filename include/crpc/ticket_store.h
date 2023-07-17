#pragma once

#include "cista/containers/hash_map.h"
#include <iostream>
#include <future>

template<typename SerializedContainer>
class ticket_store {
  using benchmark_time_unit = std::chrono::milliseconds;
  using clock = std::chrono::steady_clock;
public:
    ticket_store() : ticket_num_(0) {}

    void setValue(uint64_t ticket_number,
                  const SerializedContainer& payload)
    {
      std::lock_guard<std::mutex> lock(mutex_);
      tickets_.at(ticket_number).set_value(payload);
      tickets_.erase(ticket_number);
    }

    uint64_t nextNumber()
    {
      return ++ticket_num_;
    }

    std::future<SerializedContainer> emplace(uint64_t ticket_number)
    {
      std::promise<SerializedContainer> promise;
      auto future = promise.get_future();
      std::lock_guard<std::mutex> lock(mutex_);
      while(!tickets_.emplace(ticket_number, std::move(promise)).second) {
        std::cerr << "full at size " << tickets_.size() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      return future;
    }

private:
  std::atomic<uint64_t> ticket_num_;
  cista::raw::hash_map<uint64_t, std::promise<SerializedContainer>> tickets_;
  std::mutex mutex_;
};