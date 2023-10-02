#pragma once

#include "benchmark_interface.h"

#include "crpc/no_network/no_network_client.h"
#include "crpc/no_network/no_network_server.h"

#include "crpc/serialization/cista.h"

#include <future>
#include <fstream>

#define DATASIZE 1024 * 64

template<typename Interface, typename Serializer, bool isFloodBench, template <typename, typename> typename Server, template <typename, typename> typename Client>
class Bench {
private:
  typedef Server<Interface, Serializer> ServerType;
  typedef Client<Interface, Serializer> ClientType;
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
  auto benchmarkFunction(auto& client) {
    if constexpr (std::is_same_v<Serializer, CistaSerialzer>) {
      if constexpr (funcNum == 0)
        return client->call(&benchmark_interface<CistaSerialzer>::say_hello,
                            cista::offset::string("peter"));
      else if constexpr (funcNum == 1)
        return client->call(&benchmark_interface<CistaSerialzer>::average,
                            cista::offset::vector<double>(DATASIZE / sizeof(double)));
      else if constexpr (funcNum == 2)
        return client->call(&benchmark_interface<CistaSerialzer>::get_rand_nums,
                            int(DATASIZE / sizeof(double)));
      else if constexpr (funcNum == 3)
        return client->call(
            &benchmark_interface<CistaSerialzer>::send_rcv_large_data,
            cista::offset::vector<unsigned char>(DATASIZE / sizeof(unsigned char)));
    }
    else {
      if constexpr (funcNum == 0)
        return client->call(&benchmark_interface<Serializer>::say_hello,
                            std::string("peter"));
      else if constexpr (funcNum == 1)
        return client->call(&benchmark_interface<Serializer>::average,
                            std::vector<double>(DATASIZE / sizeof(double)));
      else if constexpr (funcNum == 2)
        return client->call(&benchmark_interface<Serializer>::get_rand_nums,
                            int(DATASIZE / sizeof(double)));
      else if constexpr (funcNum == 3)
        return client->call(
            &benchmark_interface<Serializer>::send_rcv_large_data,
            std::vector<unsigned char>(DATASIZE / sizeof(unsigned char)));
    }
  }

  template <int funcNum>
  auto getFloodBenchFunction(std::unique_ptr<ClientType>& client) {
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
  auto getLatencyBenchFunction(std::unique_ptr<ClientType>& client) {
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

  std::unique_ptr<ServerType> server;
  std::vector<std::unique_ptr<Client<Interface, Serializer>>> clients;
  int requests_per_client;
  std::vector<std::vector<std::chrono::nanoseconds>> latencies;
};

template<typename Interface, typename Serializer, bool isFloodBench, template <typename, typename> typename Server, template <typename, typename> typename Client>
void Bench<Interface, Serializer, isFloodBench, Server, Client>::writeLatencies(
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

template<typename Interface, typename Serializer, bool isFloodBench, template <typename, typename> typename Server, template <typename, typename> typename Client>
void Bench<Interface, Serializer, isFloodBench, Server, Client>::setStateCounter(auto& state) const {
  state.counters["Requests"] = clients.size() * requests_per_client;
  state.counters["Req_per_second"] = benchmark::Counter(
      clients.size() * requests_per_client, benchmark::Counter::kIsRate);
}

template<typename Interface, typename Serializer, bool isFloodBench, template <typename, typename> typename Server, template <typename, typename> typename Client>
template <int funcNum>
void Bench<Interface, Serializer, isFloodBench, Server, Client>::run(auto& state) {
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

template<typename Interface, typename Serializer, bool isFloodBench, template <typename, typename> typename Server, template <typename, typename> typename Client>
Bench<Interface, Serializer, isFloodBench, Server, Client>::Bench(int server_concurrency, int client_concurrency) {
  startServer(server_concurrency);
  buildClients(client_concurrency);
  const int requests = 1 * 1024;
  requests_per_client = requests / client_concurrency;
}

template<typename Interface, typename Serializer, bool isFloodBench, template <typename, typename> typename Server, template <typename, typename> typename Client>
Bench<Interface, Serializer, isFloodBench, Server, Client>::~Bench() {
  for (auto& client:clients)
    client->stop();
  server->stop();
}

template<typename Interface, typename Serializer, bool isFloodBench, template <typename, typename> typename Server, template <typename, typename> typename Client>
void Bench<Interface, Serializer, isFloodBench, Server, Client>::startServer(const int server_concurrency) {
  server = std::make_unique<ServerType>();
  register_benchmark_interface<ServerType, Serializer>(*server);
  server->run(server_concurrency);
}

template<typename Interface, typename Serializer, bool isFloodBench, template <typename, typename> typename Server, template <typename, typename> typename Client>
void Bench<Interface, Serializer, isFloodBench, Server, Client>::buildClients(const int client_concurrency) {
  if constexpr (std::is_same_v<no_network_server<Interface, Serializer>, Server<Interface, Serializer>>) {
    auto const transportLambda = [this](auto message, auto rcv) {
      server->receive(std::move(message), rcv);
    };

    for(int i=0; i<client_concurrency; ++i) {
      clients.push_back(std::make_unique<ClientType>(transportLambda));
    }
  }
  else {
    for (int i = 0; i < client_concurrency; ++i) {
      clients.push_back(std::make_unique<ClientType>());
    }
  }
}

template<template <typename, typename> typename Server, template <typename, typename> typename Client, template<typename> typename Interface, typename Serializer>
using FloodBench = Bench<Interface<Serializer>, Serializer, true, Server, Client>;

template<template <typename, typename> typename Server, template <typename, typename> typename Client, template<typename> typename Interface, typename Serializer>
using LatencyBench = Bench<Interface<Serializer>, Serializer, false, Server, Client>;