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

#include <fstream>

#define DATASIZE 1024 * 32

template<template <typename> typename Server, template <typename> typename Client, typename Interface, bool isFloodBench>
class Bench {
public:
  Bench(int, int);

  template <int funcNum>
  void run(auto& state);

  ~Bench();

  void writeLatencies(std::string);

protected:
  void startServer(int);
  void buildClients(int);

  void setStateCounter(auto& state) const;

  template <int funcNum>
  auto getFloodBenchFunction(std::unique_ptr<Client<Interface>>& client) {
    return [&client, this]() {
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

  template <int funcNum>
  auto getLatencyBenchFunction(std::unique_ptr<Client<Interface>>& client) {
    return [&client, this]() {
      std::vector<std::chrono::nanoseconds> times;
      times.reserve(requests_per_client);
      for (int i = 0; i < requests_per_client; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        benchmarkFunction<funcNum>(client)();
        auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
        times.push_back(dur);
      }
      return times;
    };
  }

  std::unique_ptr<Server<Interface>> server;
  std::vector<std::unique_ptr<Client<Interface>>> clients;
  int requests_per_client;
  std::vector<std::vector<std::chrono::nanoseconds>> latencies;
};

template <template <typename> typename Server, template <typename> typename Client, typename Interface, bool isFloodBench>
void Bench<Server, Client, Interface, isFloodBench>::writeLatencies(
    std::string filename) {

  std::fstream file(filename, std::ios_base::out);
  file << "[";
  for(int n=0; n < latencies.size(); ++n) {
    auto lv = latencies[n];
    file << "[";
    for(int i=0; i < lv.size() -1; ++i) {
      file << lv[i].count() << ", ";
    }
    file << lv.back().count() << "]";
    if (n != latencies.size() - 1)
      file << ",";
  }
  file << "]";

}

template <template <typename> typename Server, template <typename> typename Client, typename Interface, bool isFloodBench>
void Bench<Server, Client, Interface, isFloodBench>::setStateCounter(auto& state) const {
  state.counters["Requests"] = clients.size() * requests_per_client;
  state.counters["Req_per_second"] = benchmark::Counter(
      clients.size() * requests_per_client, benchmark::Counter::kIsRate);
}

template <template <typename> typename Server, template <typename> typename Client, typename Interface, bool isFloodBench>
template <int funcNum>
void Bench<Server, Client, Interface, isFloodBench>::run(auto& state) {
  if constexpr (funcNum != 0)
    requests_per_client /= 8;

  setStateCounter(state);

  if constexpr (isFloodBench) {
    std::vector<std::future<void>> clientRes;
    clientRes.reserve(clients.size());

    for (auto _ : state) {
      for (auto& client : clients) {
        clientRes.push_back(
            std::async(std::launch::async, getFloodBenchFunction<funcNum>(client)));
      }

      for (auto& clf : clientRes) {
        clf.wait();
      }
      clientRes.clear();
    }
  } else {
    std::vector<std::future<std::vector<std::chrono::nanoseconds>>> clientRes;
    clientRes.reserve(clients.size());

    for (auto _ : state) {
      for (auto& client : clients) {
        clientRes.push_back(
            std::async(std::launch::async, getLatencyBenchFunction<funcNum>(client)));
      }

      for (auto& clf : clientRes) {
        this->latencies.push_back(clf.get());
      }
      clientRes.clear();
    }
  }
}

template<template <typename> typename Server, template <typename> typename Client, typename Interface, bool isFloodBench>
Bench<Server, Client, Interface, isFloodBench>::Bench(int server_concurrency, int client_concurrency) {
  startServer(server_concurrency);
  buildClients(client_concurrency);
  const int requests = 64 * 1024;
  requests_per_client = requests / client_concurrency;
}

template <template <typename> typename Server, template <typename> typename Client, typename Interface, bool isFloodBench>
Bench<Server, Client, Interface, isFloodBench>::~Bench() {
  for (auto& client:clients)
    client->stop();
  server->stop();
}

template<template <typename> typename Server, template <typename> typename Client, typename Interface, bool isFloodBench>
void Bench<Server, Client, Interface, isFloodBench>::startServer(const int server_concurrency) {
  server = std::make_unique<Server<Interface>>();
  register_benchmark_interface(*server);
  server->run(server_concurrency);
}

template<template <typename> typename Server, template <typename> typename Client, typename Interface, bool isFloodBench>
void Bench<Server, Client, Interface, isFloodBench>::buildClients(const int client_concurrency) {
  for(int i=0; i<client_concurrency; ++i) {
    clients.push_back(std::make_unique<Client<Interface>>());
  }
}

template<>
void Bench<no_network_server, no_network_client, benchmark_interface, true>::buildClients(const int client_concurrency) {
  std::function<void(const std::vector<uint8_t>, std::function<void(const std::vector<uint8_t>)>)> const transportLambda = [this](const std::vector<uint8_t>& message, auto rcv) { server->receive(std::move(message), rcv); };
  for(int i=0; i<client_concurrency; ++i) {
    clients.push_back(std::make_unique<no_network_client<benchmark_interface>>(transportLambda));
  }
}

template<>
void Bench<no_network_server, no_network_client, benchmark_interface, false>::buildClients(const int client_concurrency) {
  std::function<void(const std::vector<uint8_t>, std::function<void(const std::vector<uint8_t>)>)> const transportLambda = [this](const std::vector<uint8_t>& message, auto rcv) { server->receive(std::move(message), rcv); };
  for(int i=0; i<client_concurrency; ++i) {
    clients.push_back(std::make_unique<no_network_client<benchmark_interface>>(transportLambda));
  }
}

template<template <typename> typename Server, template <typename> typename Client, typename Interface>
using FloodBench = Bench<Server, Client, Interface, true>;

template<template <typename> typename Server, template <typename> typename Client, typename Interface>
using LatencyBench = Bench<Server, Client, Interface, false>;


template<int funcNum>
auto benchmarkFunction(auto& client) {
  if constexpr (funcNum == 0)
    return client->call(&benchmark_interface::say_hello,
              data::string("peter"));
  else if constexpr (funcNum == 1)
    return client->call(&benchmark_interface::average,
                 data::vector<double>(DATASIZE / sizeof(double)));
  else if constexpr (funcNum == 2)
    return client->call(&benchmark_interface::get_rand_nums,
                        int(DATASIZE / sizeof(double)));
  else if constexpr (funcNum == 3)
    return client->call(&benchmark_interface::send_rcv_large_data,
                 data::vector<unsigned char>(DATASIZE / sizeof(unsigned char)));
}

template <int funcNum>
static void BM_NoNetwork(benchmark::State& state) {
  FloodBench<no_network_server, no_network_client, benchmark_interface> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_HTTP(benchmark::State& state) {
  FloodBench<http_ws_server, rpc_http_client, benchmark_interface> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_WS(benchmark::State& state) {
  FloodBench<http_ws_server, rpc_ws_client, benchmark_interface> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_MQTT(auto& state) {
  FloodBench<rpc_mqtt_server, rpc_mqtt_client, benchmark_interface> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}


template <int funcNum>
static void BM_Latency_NoNetwork(benchmark::State& state) {
  LatencyBench<http_ws_server, rpc_http_client, benchmark_interface> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_NoNetwork_<" + std::to_string(funcNum) + ">.txt");
}

template <int funcNum>
static void BM_Latency_HTPP(benchmark::State& state) {
  LatencyBench<no_network_server, no_network_client, benchmark_interface> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_HTPP_<" + std::to_string(funcNum) + ">.txt");
}

template <int funcNum>
static void BM_Latency_WS(benchmark::State& state) {
  LatencyBench<http_ws_server, rpc_ws_client, benchmark_interface> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_WS_<" + std::to_string(funcNum) + ">.txt");
}

template <int funcNum>
static void BM_Latency_MQTT(benchmark::State& state) {
  LatencyBench<rpc_mqtt_server, rpc_mqtt_client, benchmark_interface> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_MQTT_<" + std::to_string(funcNum) + ">.txt");
}



static void FloodBenchArguments(benchmark::internal::Benchmark* b) {
  b->UseRealTime()->ArgsProduct({benchmark::CreateRange(1, 128, 2), benchmark::CreateRange(1, 128, 2)})->Unit(benchmark::kMillisecond);
}

static void LatencyBenchArguments(benchmark::internal::Benchmark* b) {
  b->UseRealTime()->Iterations(1)->ArgPair(1, 1)->Unit(benchmark::kMillisecond);
}


BENCHMARK(BM_HTTP<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_HTTP<1>)->Apply(FloodBenchArguments);
BENCHMARK(BM_HTTP<2>)->Apply(FloodBenchArguments);
BENCHMARK(BM_HTTP<3>)->Apply(FloodBenchArguments);

BENCHMARK(BM_WS<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_WS<1>)->Apply(FloodBenchArguments);
BENCHMARK(BM_WS<2>)->Apply(FloodBenchArguments);
BENCHMARK(BM_WS<3>)->Apply(FloodBenchArguments);

BENCHMARK(BM_MQTT<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_MQTT<1>)->Apply(FloodBenchArguments);
BENCHMARK(BM_MQTT<2>)->Apply(FloodBenchArguments);
BENCHMARK(BM_MQTT<3>)->Apply(FloodBenchArguments);


BENCHMARK(BM_NoNetwork<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork<1>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork<2>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork<3>)->Apply(FloodBenchArguments);


BENCHMARK(BM_Latency_NoNetwork<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork<3>)->Apply(LatencyBenchArguments);

BENCHMARK(BM_Latency_HTPP<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_HTPP<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_HTPP<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_HTPP<3>)->Apply(LatencyBenchArguments);

BENCHMARK(BM_Latency_WS<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_WS<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_WS<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_WS<3>)->Apply(LatencyBenchArguments);

BENCHMARK(BM_Latency_MQTT<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_MQTT<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_MQTT<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_MQTT<3>)->Apply(LatencyBenchArguments);


BENCHMARK_MAIN();