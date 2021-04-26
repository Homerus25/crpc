#pragma once

#include "crpc/rpc_client.h"
#include <vector>
#include <iostream>

namespace data = cista::offset;

struct benchmark_interface {
  fn<data::string, data::string> say_hello;
  fn<double, data::vector<double>> average;
  fn<data::vector<double>, int> get_rand_nums;
  fn<data::vector<char>, data::vector<char>> send_rcv_large_data;
};

inline void wait_for_start_signal() {
  //std::cout << "wait for signal" << std::endl;
  char unused;
  std::cin >> unused;
}

template<typename Server>
void register_benchmark_interface(Server &server) {
  server.reg(&benchmark_interface::say_hello, [](data::string ms){
    auto str = std::string("hello " + ms.str());
    //std::cout << "send: " << str << "\n";
    return data::string(str);
  });
  server.reg(&benchmark_interface::average,
             [](const data::vector<double>& vec) {
               double avg = 0.;
               for(double element : vec)
                 avg += element / vec.size();
               return avg;
  });
  server.reg(&benchmark_interface::get_rand_nums,
             [](int size) {
               data::vector<double> ret(size);
               std::iota(ret.begin(), ret.end(), 0.);
               return ret;
  });
  server.reg(&benchmark_interface::send_rcv_large_data, [](data::vector<char> l_data) { return l_data; });
}

template<typename Client>
void benchmark_run(Client& client, int times)
{
  wait_for_start_signal();
  std::vector<decltype(client.call(&benchmark_interface::say_hello, data::string()))> results_say_hello;
  std::vector<decltype(client.call(&benchmark_interface::average, {}))> results_average;
  std::vector<decltype(client.call(&benchmark_interface::get_rand_nums, 1))> results_get_rand_numbers;
  std::vector<decltype(client.call(&benchmark_interface::send_rcv_large_data, data::vector<char>()))> results_send_rcv_large_data;
  auto start = std::chrono::high_resolution_clock::now();
  for(auto i = 0; i < times; ++i) {
    results_say_hello.emplace_back(client.call(&benchmark_interface::say_hello, data::string("peter")));
    results_average.emplace_back(client.call(&benchmark_interface::average, {0., 10., 100., 1000.}));
    results_get_rand_numbers.emplace_back(client.call(&benchmark_interface::get_rand_nums, 100));
    results_send_rcv_large_data.emplace_back(client.call(&benchmark_interface::send_rcv_large_data, data::vector<char>(1000)));
  }

  for(int i = 0; i < times; ++i) {
    results_say_hello[i]();
    results_average[i]();
    results_get_rand_numbers[i]();
    results_send_rcv_large_data[i]();
  }

  auto end = std::chrono::high_resolution_clock::now();
  //std::cout << "time: " << duration_cast<std::chrono::milliseconds>(end - start).count() << "\n";
  std::cout << duration_cast<std::chrono::milliseconds>(end - start).count();// << "\n";
}


template<typename ReturnType, typename ...ArgTypes>
void benchmark_run_generic(std::function<ReturnType(ArgTypes...)> proc, int times) {
  wait_for_start_signal();
  std::vector<ReturnType> results;
  results.reserve(times);

  auto start = std::chrono::high_resolution_clock::now();

  for(auto i = 0; i < times; ++i)
    results.emplace_back(proc());

  for(int i = 0; i < times; ++i) {
    auto res = results[i]();
    //std::cout << "got: " << res << std::endl;
    if (res != "hello peter")
      std::cerr << "invalid result!" << res << "\n";
  }
  //std::cout << "finished!" << std::endl;

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << duration_cast<std::chrono::milliseconds>(end - start).count();// << "\n";
}

template<typename Client>
void benchmark_run_say_hello(Client& client, int times) {
  std::function proc = [&](){return client.call(&benchmark_interface::say_hello, data::string("peter"));};
  benchmark_run_generic(proc, times);
}
