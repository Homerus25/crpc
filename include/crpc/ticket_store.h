#pragma once

#include "cista/serialization.h"
#include <iostream>
#include <future>

class ticket_store {
  using benchmark_time_unit = std::chrono::milliseconds;
  using clock = std::chrono::steady_clock;
public:
    ticket_store() : ticket_num_(0) {}

    void setValue(uint64_t ticket_number,
                  const cista::offset::vector<unsigned char>& payload)
    {
      std::lock_guard<std::mutex> lock(mutex_);
      tickets_.at(ticket_number).set_value(std::vector<unsigned char>(payload.begin(), payload.end()));
      tickets_.erase(ticket_number);
    }

    uint64_t nextNumber()
    {
      return ++ticket_num_;
    }

    std::future<std::vector<unsigned char>> emplace(uint64_t ticket_number)
    {
      std::lock_guard<std::mutex> lock(mutex_);
      std::promise<std::vector<unsigned char>> promise;
      auto future = promise.get_future();
      while(!tickets_.emplace(ticket_number, std::move(promise)).second) {
        std::cerr << "full at size " << tickets_.size() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      return future;
    }

private:
  std::atomic<uint64_t> ticket_num_;
  cista::raw::hash_map<uint64_t, std::promise<std::vector<unsigned char>>> tickets_;
  std::mutex mutex_;
};