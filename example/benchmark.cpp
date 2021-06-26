/*
#include "benchmark.h"

benchmark::benchmark(int clients_count, std::function<void()> cf) {

  this->start_point = std::latch(clients_count);

  this->clients.reserve(clients_count);
  while(this->clients.size() < clients_count) {
    this->clients.emplace_back(cf);
  }
}

void benchmark::wait_start() {
  this->start_point.wait_and_arrive();
}

void benchmark::save_time(int time) {
  std::lock_guard lg(this->save_mutex);
  this->rtts.push_back(time);
}
*/