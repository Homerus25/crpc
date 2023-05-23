#pragma once

#include <shared_mutex>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <vector>
#include <functional>

class benchmark_custom {
public:
  benchmark_custom(int, std::function<void()>);

  void wait_start();

  void save_time(std::vector<std::chrono::milliseconds>& new_times);

private:
  //std::latch start_point;
  //std::condition_variable start_point;
  std::vector<std::thread> clients;
  std::mutex save_mutex;
  std::vector<int> rtts;

  //std::atomic<int> arrived_threads;
  int arrived_threads;
  std::shared_mutex start_mutex;
  std::condition_variable_any start_cv;
  bool all_waited;
};

benchmark_custom::benchmark_custom(int clients_count, std::function<void()> cf)
    //: start_point()
  //: start_point(clients_count)
  : arrived_threads(0)
  , all_waited(false)
{

  //std::cout << "start " << clients_count << std::endl;
  this->clients.reserve(clients_count);
  while(this->clients.size() < clients_count) {
    this->clients.emplace_back(cf);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  //std::cout << "started" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  while(this->arrived_threads < clients_count) {
    //std::cout << "clients: " << this->arrived_threads << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  all_waited = true;

  //std::cout << "start all!" << std::endl;
  start_cv.notify_all();
  //std::cout << "all notified!" << std::endl;

  for(auto &client : this->clients) {
    client.join();
  }

  //std::cout << "results:";
  for(auto time : this->rtts) {
    std::cout << time << " ";
  }
}

void benchmark_custom::wait_start() {
  //this->start_point.arrive_and_wait();
  {
    std::unique_lock<std::shared_mutex> l(this->start_mutex);
    ++this->arrived_threads;
  }
  std::shared_lock<std::shared_mutex> l(this->start_mutex);
  start_cv.wait(l, [&]{ return this->all_waited; });
}

void benchmark_custom::save_time(std::vector<std::chrono::milliseconds>& new_times) {
  std::lock_guard lg(this->save_mutex);
  rtts.reserve(rtts.size() + new_times.size());
  for(auto time : new_times) {
    rtts.emplace_back(time.count());
  }
}
