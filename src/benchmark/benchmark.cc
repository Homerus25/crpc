#include <benchmark/benchmark.h>
#include "benchmark_custom.h"
#include "benchmark_interface.h"
#include "crpc/no_network/no_network_client.h"
#include "crpc/no_network/no_network_server.h"
#include "crpc/http/http_ws_server.h"
#include "crpc/http/rpc_http_client.h"
#include "crpc/http/rpc_ws_client.h"
#include "crpc/mqtt/rpc_mqtt_server.h"
#include "crpc/mqtt/rpc_mqtt_transport.h"


template<template <typename> typename Server, template <typename> typename Client, typename Interface>
class Bench {
public:
  Bench(int, int);

  template <int funcNum>
  void run(auto& state);

  ~Bench();

private:
  void startServer(int);
  void buildClients(int);

  void setStateCounter(auto& state) const;
  template <int funcNum>
  auto floodBench(const int requests_per_client, std::unique_ptr<Client<Interface>>& client) const {
    return [&client, requests_per_client]() {
      std::vector<decltype(benchmarkFunction<funcNum>(client))> resp;
      resp.reserve(requests_per_client);
      for (int i = 0; i < requests_per_client; ++i) {
        resp.push_back(benchmarkFunction<funcNum>(client));
      }

      for (auto& re : resp) {
        re();
      }
    };
  }

  std::unique_ptr<Server<Interface>> server;
  std::vector<std::unique_ptr<Client<Interface>>> clients;
  int requests_per_client;
};

template <template <typename> typename Server, template <typename> typename Client, typename Interface>
void Bench<Server, Client, Interface>::setStateCounter(auto& state) const {
  state.counters["Requests"] = clients.size() * requests_per_client;
  state.counters["Req_per_second"] = benchmark::Counter(
      clients.size() * requests_per_client, benchmark::Counter::kIsRate);
}

template <template <typename> typename Server, template <typename> typename Client, typename Interface>
template <int funcNum>
void Bench<Server, Client, Interface>::run(auto& state) {
  setStateCounter(state);

  std::vector<std::future<void>> clientRes;
  clientRes.reserve(clients.size());

  for (auto _ : state) {
    for(auto& client : clients) {
      clientRes.push_back(std::async(std::launch::async,
                                     floodBench<funcNum>(requests_per_client, client)));
    }

    for (auto& clf: clientRes) {
      clf.wait();
    }
    clientRes.clear();
  }
}

template<template <typename> typename Server, template <typename> typename Client, typename Interface>
Bench<Server, Client, Interface>::Bench(int server_concurrency, int client_concurrency) {
  startServer(server_concurrency);
  buildClients(client_concurrency);
  const int requests = 64 * 1024;
  requests_per_client = requests / client_concurrency;
}

template <template <typename> typename Server, template <typename> typename Client, typename Interface>
Bench<Server, Client, Interface>::~Bench() {
  for (auto& client:clients)
    client->stop();
  server->stop();
}

template<template <typename> typename Server, template <typename> typename Client, typename Interface>
void Bench<Server, Client, Interface>::startServer(const int server_concurrency) {
  server = std::make_unique<Server<Interface>>();
  register_benchmark_interface(*server);
  server->run(server_concurrency);
}

template<template <typename> typename Server, template <typename> typename Client, typename Interface>
void Bench<Server, Client, Interface>::buildClients(const int client_concurrency) {
  for(int i=0; i<client_concurrency; ++i) {
    clients.push_back(std::make_unique<Client<Interface>>());
  }
}

template<>
void Bench<no_network_server, no_network_client, benchmark_interface>::buildClients(const int client_concurrency) {
  std::function<void(const std::vector<uint8_t>, std::function<void(const std::vector<uint8_t>)>)> const transportLambda = [this](const std::vector<uint8_t>& message, auto rcv) { server->receive(std::move(message), rcv); };
  for(int i=0; i<client_concurrency; ++i) {
    clients.push_back(std::make_unique<no_network_client<benchmark_interface>>(transportLambda));
  }
}


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

template <int funcNum>
static void BM_NoNetwork(benchmark::State& state) {
  Bench<no_network_server, no_network_client, benchmark_interface> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_HTTP(benchmark::State& state) {
  Bench<http_ws_server, rpc_http_client, benchmark_interface> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_WS(benchmark::State& state) {
  Bench<http_ws_server, rpc_ws_client, benchmark_interface> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_MQTT(auto& state) {
  Bench<rpc_mqtt_server, rpc_mqtt_client, benchmark_interface> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
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