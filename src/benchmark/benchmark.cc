#include <benchmark/benchmark.h>
#include "benchmark_custom.h"
#include "benchmark_interface.h"
#include "crpc/no_network/no_network_client.h"
#include "crpc/no_network/no_network_server.h"

template<int funcNum>
auto benchmarkFunction(auto& client) {
  if constexpr (funcNum == 0)
    return client->call(&benchmark_interface::say_hello,
              data::string("peter"));
  else if constexpr (funcNum == 1)
    return client->call(&benchmark_interface::average,
                 data::vector<double>(100));
  else if constexpr (funcNum == 2)
    return client->call(&benchmark_interface::get_rand_nums,
                 100);
  else if constexpr (funcNum == 3)
    return client->call(&benchmark_interface::send_rcv_large_data,
                 data::vector<unsigned char>(1000));
}


void run_conncurent(boost::asio::io_context& ioc, int thread_count) {
  for(int i=0; i<thread_count; ++i) {
    std::thread([&ioc](){ioc.run();}).detach();
  }
}

template <int funcNum>
static void BM_NoNetwork(benchmark::State& state) {
  const int server_concurrency = state.range(0);
  const int client_concurrency = state.range(1);

  // start server
  boost::asio::io_context server_ioc(server_concurrency);
  auto server = no_network_server<benchmark_interface>(server_ioc);
  register_benchmark_interface(server);
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> x
      = boost::asio::make_work_guard(server_ioc);
  run_conncurent(server_ioc, server_concurrency);

  // build clients
  std::vector<std::unique_ptr<no_network_client<benchmark_interface>>> clients;
  std::function<void(const std::vector<uint8_t>, std::function<void(const std::vector<uint8_t>)>)> const transportLambda = [&server](const std::vector<uint8_t>& message, auto rcv) { server.receive(message, rcv); };
  for(int i=0; i<client_concurrency; ++i) {
    clients.push_back(std::make_unique<no_network_client<benchmark_interface>>(transportLambda));
  }

  doBench<funcNum>(state, clients);

  server.stop();
}

BENCHMARK(BM_NoNetwork<0>)
    ->ArgPair(1,1)->ArgPair(2,1)/*->ArgPair(4,1)->ArgPair(8,1)
    ->ArgPair(1,2)->ArgPair(1,4)->ArgPair(1, 8)
    ->ArgPair(2,2)->ArgPair(4,4)->ArgPair(8,8)*/
    ->UseRealTime()/*->Unit(benchmark::kMillisecond)*/
;

/*
    ->ComputeStatistics("max", [](const std::vector<double>& v) -> double {
      return *(std::max_element(std::begin(v), std::end(v)));
    })
    ->ComputeStatistics("time_per_100", [](const std::vector<double>& v) -> double {
      double tim=0;
      std::ranges::for_each(v, [&](auto val) { tim += val; });
      std::cout << v.size() << std::endl;
      return tim / v.size();
    });
    //->Repetitions(5);
*/


#include "crpc/http/http_ws_server.h"
#include "crpc/http/rpc_http_client.h"

template <int funcNum>
void doBench(benchmark::State& state, auto& clients) {
  std::vector<std::future<void>> clientRes;
  clientRes.reserve(clients.size());

  for (auto _ : state) {
    for(auto& client : clients) {
      clientRes.push_back(std::async(std::launch::async, [&client]() {
        constexpr int iterations = 100;
        std::vector<decltype(benchmarkFunction<funcNum>(client))> resp;
        resp.reserve(iterations);
        for (int i = 0; i < iterations; ++i) {
          resp.push_back(benchmarkFunction<funcNum>(client));
        }

        for (auto& re : resp) {
          re();
        }
      }));
    }

    for (auto& clf: clientRes) {
      clf.wait();
    }
    clientRes.clear();
  }

  for (auto& client:clients)
    client->stop();
}


template <int funcNum>
static void BM_HTTP(benchmark::State& state) {
  const int server_concurrency = state.range(0);
  const int client_concurrency = state.range(1);

  // start server
  boost::asio::io_context server_ioc(server_concurrency);
  auto server = http_ws_server<benchmark_interface>{server_ioc};
  register_benchmark_interface(server);
  std::thread sert([&server_ioc]() {server_ioc.run();});

  // build clients
  std::vector<std::unique_ptr<rpc_http_client<benchmark_interface>>> clients;
  for(int i=0; i<client_concurrency; ++i) {
    clients.push_back(std::make_unique<rpc_http_client<benchmark_interface>>("http://127.0.0.1:9000/", 9000u));  // second parameter unused
  }

  doBench<funcNum>(state, clients);

  server.stop();
  server_ioc.stop();
  sert.join();
}

BENCHMARK(BM_HTTP<1>)
    ->ArgPair(1,1)->ArgPair(2,1)/*->ArgPair(4,1)->ArgPair(8,1)
    ->ArgPair(1,2)->ArgPair(1,4)->ArgPair(1, 8)
    ->ArgPair(2,2)->ArgPair(4,4)->ArgPair(8,8)
  */->UseRealTime();


BENCHMARK_MAIN();