#pragma once

//#include <barrier>
#include <iostream>
#include <random>
#include <vector>
#include "benchmark.h"
#include "crpc/rpc_async_client.h"
#include "crpc/rpc_client.h"

namespace data = cista::offset;

struct benchmark_interface {
  fn<data::string, data::string> say_hello;
  fn<double, data::vector<double>> average;
  fn<data::vector<double>, int> get_rand_nums;
  fn<data::vector<char>, data::vector<char>> send_rcv_large_data;
};

inline void wait_for_start_signal() {
  //std::cout << "wait for signal" << std::endl;
  //char unused;
  //std::cin >> unused;
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
  std::cout << duration_cast<std::chrono::milliseconds>(end - start).count();// << "\n";
}


template<typename ReturnType>
void benchmark_run_generic(std::function<ReturnType(void)> proc, benchmark& bench, int iterations) {
  std::vector<ReturnType> results;
  results.reserve(iterations);

  bench.wait_start();

  for(auto i = 0; i < iterations; ++i)
    results.emplace_back(proc());

  for(int i = 0; i < iterations; ++i) {
    auto res = results[i]();
  }
}

template<typename Client>
void benchmark_run_say_hello(Client& client, benchmark& bench, int iterations) {
  std::function proc = [&](){return client.call(&benchmark_interface::say_hello, data::string("peter"));};
  benchmark_run_generic(proc, bench, iterations);
}

template<typename Client>
void benchmark_average(Client& client, benchmark& bench, int iterations, int size) {
  std::function<return_object<double>()> proc = [&client, &size]{return client.call(&benchmark_interface::average, data::vector<double>(size));};
  benchmark_run_generic(proc, bench, iterations);
}

template<typename Client>
void benchmark_get_random_numbers(Client& client, benchmark& bench, int iterations, int size) {
  std::function<return_object<data::vector<double>>()> proc = [&client,size](){return client.call(&benchmark_interface::get_rand_nums, int(size));};
  benchmark_run_generic(proc, bench, iterations);
}

template<typename Client>
void benchmark_send_receive(Client& client, benchmark& bench, int iterations, int size) {
  std::function proc = [&](){return client.call(&benchmark_interface::send_rcv_large_data, data::vector<char>(size));};
  benchmark_run_generic(proc, bench, iterations);
}

struct BenchmarkParameter {
  int functionID;
  int clientCount;
  int iterations;
  int benchsize;
};

std::optional<BenchmarkParameter> parse_benchmark_parameters(int argc, char* argv[]) {
  if(argc == 4 || argc == 5) {
    int clientsCount = std::atoi(argv[1]);
    int fID = std::atoi(argv[2]);
    int iterations = std::atoi(argv[3]);

    int benchsize;
    if(argc == 5) {
      benchsize = std::atoi(argv[4]);
    }
    return std::optional(BenchmarkParameter{fID, clientsCount, iterations, benchsize});
  }
  std::cout << "wrong paramters!" << std::endl;
  return {};
}

template<typename Client>
std::function<void()> get_benchmark_function(Client& client, benchmark& bench, BenchmarkParameter& parameter) {
  switch (parameter.functionID) {
    case 0: return [&client, iterations = parameter.iterations]() { benchmark_run(client, iterations); };
      break;

    case 1: return [&client, &bench, iterations = parameter.iterations]() { benchmark_run_say_hello(client, bench, iterations); };
      break;

    case 2: return [&client, &bench, iterations = parameter.iterations, size = parameter.benchsize]() { benchmark_average(client, bench, iterations, size); };
      break;

    case 3: return [&client, &bench, iterations = parameter.iterations, size = parameter.benchsize]() { benchmark_get_random_numbers(client, bench, iterations, size); };
      break;

    case 4: return [&client, &bench, iterations = parameter.iterations, size = parameter.benchsize]() { benchmark_send_receive(client, bench, iterations, size); };
      break;
  }
}