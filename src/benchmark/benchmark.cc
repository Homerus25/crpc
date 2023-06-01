#include <benchmark/benchmark.h>
#include "benchmark_custom.h"
#include "benchmark_interface.h"
#include "crpc/no_network/no_network_client.h"
#include "crpc/no_network/no_network_server.h"

#define WORK_ITERATIONS 1024

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
void doBench(auto& state, auto& clients) {
  std::vector<std::future<void>> clientRes;
  clientRes.reserve(clients.size());

  for (auto _ : state) {
    for(auto& client : clients) {
      clientRes.push_back(std::async(std::launch::async, [&client]() {
        constexpr int iterations = WORK_ITERATIONS;
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
static void BM_NoNetwork(benchmark::State& state) {
  const int server_concurrency = state.range(0);
  const int client_concurrency = state.range(1);

  // start server
  auto server = no_network_server<benchmark_interface>(/*server_ioc*/);
  register_benchmark_interface(server);
  server.run(server_concurrency);

  // build clients
  std::vector<std::unique_ptr<no_network_client<benchmark_interface>>> clients;
  std::function<void(const std::vector<uint8_t>, std::function<void(const std::vector<uint8_t>)>)> const transportLambda = [&server](const std::vector<uint8_t>& message, auto rcv) { server.receive(std::move(message), rcv); };
  for(int i=0; i<client_concurrency; ++i) {
    clients.push_back(std::make_unique<no_network_client<benchmark_interface>>(transportLambda));
  }

  doBench<funcNum>(state, clients);

  server.stop();

  state.counters["Requests"] = client_concurrency * WORK_ITERATIONS;
  state.counters["Req_per_second"] = benchmark::Counter(client_concurrency * WORK_ITERATIONS, benchmark::Counter::kIsRate);
}


#include "crpc/http/http_ws_server.h"
#include "crpc/http/rpc_http_client.h"




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


#include "crpc/http/rpc_ws_client.h"

template <int funcNum>
static void BM_WS(benchmark::State& state) {
  const int server_concurrency = state.range(0);
  const int client_concurrency = state.range(1);

  // start server
  boost::asio::io_context server_ioc(server_concurrency);
  auto server = http_ws_server<benchmark_interface>{server_ioc};
  register_benchmark_interface(server);
  std::thread sert([&server_ioc]() {server_ioc.run();});

  // build clients
  std::vector<std::unique_ptr<rpc_ws_client<benchmark_interface>>> clients;
  for(int i=0; i<client_concurrency; ++i) {
    clients.push_back(std::make_unique<rpc_ws_client<benchmark_interface>>("127.0.0.1", 9000u));
  }

  doBench<funcNum>(state, clients);

  server.stop();
  server_ioc.stop();
  sert.join();
}


#include "crpc/mqtt/rpc_mqtt_server.h"
#include "crpc/mqtt/rpc_mqtt_transport.h"

template <int funcNum>
static void BM_MQTT(auto& state) {
  const int server_concurrency = state.range(0);
  const int client_concurrency = state.range(1);

  // start server
  auto server = rpc_mqtt_server<benchmark_interface>{2000};
  register_benchmark_interface(server);
  server.run(server_concurrency);

  // build clients
  std::vector<std::unique_ptr<rpc_mqtt_client<benchmark_interface>>> clients;
  for(int i=0; i<client_concurrency; ++i) {
    clients.push_back(std::make_unique<rpc_mqtt_client<benchmark_interface>>(std::string("127.0.0.1"), static_cast<std::uint16_t>(2000)));
  }

  doBench<funcNum>(state, clients);

  server.stop();
}


static void CustomArguments(benchmark::internal::Benchmark* b) {
  b->UseRealTime()->ArgsProduct({benchmark::CreateRange(1, 64, 2), benchmark::CreateRange(1, 64, 2)});
}


BENCHMARK(BM_HTTP<0>)->Apply(CustomArguments);
BENCHMARK(BM_HTTP<1>)->Apply(CustomArguments);
BENCHMARK(BM_HTTP<2>)->Apply(CustomArguments);
BENCHMARK(BM_HTTP<3>)->Apply(CustomArguments);

BENCHMARK(BM_WS<0>)->Apply(CustomArguments);
BENCHMARK(BM_WS<1>)->Apply(CustomArguments);
BENCHMARK(BM_WS<2>)->Apply(CustomArguments);
BENCHMARK(BM_WS<3>)->Apply(CustomArguments);

BENCHMARK(BM_MQTT<0>)->Apply(CustomArguments);
BENCHMARK(BM_MQTT<1>)->Apply(CustomArguments);
BENCHMARK(BM_MQTT<2>)->Apply(CustomArguments);
BENCHMARK(BM_MQTT<3>)->Apply(CustomArguments);


BENCHMARK(BM_NoNetwork<0>)->Apply(CustomArguments);
BENCHMARK(BM_NoNetwork<1>)->Apply(CustomArguments);
BENCHMARK(BM_NoNetwork<2>)->Apply(CustomArguments);
BENCHMARK(BM_NoNetwork<3>)->Apply(CustomArguments);


BENCHMARK_MAIN();