#pragma once

#include <string>
#include <memory>
#include <benchmark/benchmark.h>

#include "benchmark_interface.h"
#include "crpc/no_network/no_network_client.h"
#include "crpc/no_network/no_network_server.h"

template<typename ServerType, typename ClientType>
struct IdentitySerializer {
  typedef ServerType SerializedServerContainer;
  typedef message<SerializedServerContainer> SerializedServerMessageContainer;
  typedef ClientType SerializedClientContainer;
  typedef message<SerializedClientContainer> SerializedClientMessageContainer;

  template <class In>
  static auto serialize(In& in) {
    if constexpr (std::is_same_v<std::tuple<SerializedServerContainer>, In>) {
      return std::get<SerializedServerContainer>(in);
    }
    else if constexpr (std::is_same_v<std::tuple<SerializedClientContainer>, In>) {
      return std::get<SerializedClientContainer>(in);
    }
    else
      return in;
  }

  template <class Out, class In>
  static auto deserialize(In& in) {
    return std::make_unique<Out>(std::move(in));
  }
};

template<typename RetType, typename ParamType>
static void BM_NoNet_NoSerial_Latency_Template(benchmark::State& state, auto regFn, auto benchFunc, std::string filename) {
  typedef IdentitySerializer<RetType, ParamType> Serializer;
  typedef benchmark_interface<Serializer> InterfaceT;
  typedef no_network_server<InterfaceT, Serializer> ServerT;
  std::unique_ptr<ServerT> server = std::make_unique<ServerT>();

  int server_concurrency = state.range(0);
  int client_concurrency = state.range(1);

  regFn(server);
  server->run(server_concurrency);


  auto const transportLambda = [&server](auto message, auto rcv) {
    server->receive(std::move(message), rcv);
  };


  typedef no_network_client<InterfaceT, Serializer> ClientT;
  std::vector<std::unique_ptr<ClientT>> clients;
  for(int i=0; i<client_concurrency; ++i)
    clients.emplace_back(std::make_unique<ClientT>(transportLambda));

  int request_count = 64 * 1024;
  int requests_per_client = request_count / client_concurrency;

  state.counters["Requests"] = request_count;
  state.counters["Req_per_second"] = benchmark::Counter(request_count, benchmark::Counter::kIsRate);

  std::vector<std::vector<std::chrono::nanoseconds>> latencies;
  std::vector<std::future<std::vector<std::chrono::nanoseconds>>> clientRes;
  clientRes.reserve(client_concurrency);

  for (auto _ : state) {
    for (auto& client : clients) {
      clientRes.push_back(std::async(std::launch::async, [&client, &benchFunc, requests_per_client]() {
        std::vector<std::chrono::nanoseconds> times;
        times.reserve(requests_per_client);
        for (int i = 0; i < requests_per_client; ++i) {
          auto start = std::chrono::high_resolution_clock::now();
          benchFunc(client)();
          auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
          times.push_back(dur);
        }
        return times;
      }));
    }

    for (auto& clf : clientRes) {
      latencies.push_back(clf.get());
    }
    clientRes.clear();
  }

  for (auto& client:clients)
    client->stop();
  server->stop();

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


template<typename RetType, typename ParamType>
static void BM_NoNet_NoSerial_Template(benchmark::State& state, auto regFn, auto benchFunc) {
  typedef IdentitySerializer<RetType, ParamType> Serializer;
  //typedef IdentitySerializer<double> SerializerServer;
  typedef benchmark_interface<Serializer> InterfaceT;
  typedef no_network_server<InterfaceT, Serializer> ServerT;
  std::unique_ptr<ServerT> server = std::make_unique<ServerT>();

  int server_concurrency = state.range(0);
  int client_concurrency = state.range(1);

  regFn(server);
  server->run(server_concurrency);


  auto const transportLambda = [&server](auto message, auto rcv) {
    server->receive(std::move(message), rcv);
  };


  typedef no_network_client<InterfaceT, Serializer> ClientT;
  std::vector<std::unique_ptr<ClientT>> clients;
  for(int i=0; i<client_concurrency; ++i)
    clients.emplace_back(std::make_unique<ClientT>(transportLambda));

  int request_count = 256 * 1024;
  int requests_per_client = request_count / client_concurrency;

  state.counters["Requests"] = request_count;
  state.counters["Req_per_second"] = benchmark::Counter(request_count, benchmark::Counter::kIsRate);

  std::vector<std::future<void>> clientRes;
  clientRes.reserve(client_concurrency);

  for (auto _ : state) {
    for (auto& client : clients) {
      clientRes.push_back(std::async(std::launch::async, [&client, &benchFunc, requests_per_client]() {
        std::vector<decltype(benchFunc(client))> resp;
        resp.reserve(requests_per_client);
        for (int i = 0; i < requests_per_client; ++i) {
          resp.push_back(benchFunc(client));
        }

        for (auto& re : resp) {
          re();
        }
      }));
    }
    for(auto& res: clientRes) {
      res.wait();
    }
    clientRes.clear();
  }

  for (auto& client:clients)
    client->stop();
  server->stop();
}

static void BM_NoNet_NoSerial_say_hello(benchmark::State& state) {
  typedef benchmark_interface<IdentitySerializer<std::string, std::string>> InterfaceT;
  BM_NoNet_NoSerial_Template<std::string, std::string>(state, [](auto& server){
        server->reg(&InterfaceT::say_hello, [](std::string ms) {
          return std::string("hello " + ms);
        });
      },
      [](auto& client){
        return client->call(&InterfaceT::say_hello,
                            std::string("peter"));
      });
}

static void BM_NoNet_NoSerial_average(benchmark::State& state) {
  typedef benchmark_interface<IdentitySerializer<double, std::vector<double>>> InterfaceT;
  BM_NoNet_NoSerial_Template<double, std::vector<double>>(state, [](auto& server){
        server->reg(&InterfaceT::average,
          [](const std::vector<double>& vec) {
            double avg = 0.;
            for (double element : vec) avg += element / vec.size();
            return avg;
          });
      },
      [](auto& client){
        return client->call(&InterfaceT::average,
                            std::vector<double>(1024 * 64 / sizeof(double)));
      });
}

static void BM_NoNet_NoSerial_get_rand_nums(benchmark::State& state) {
  typedef benchmark_interface<IdentitySerializer<std::vector<double>, int>> InterfaceT;
  BM_NoNet_NoSerial_Template<std::vector<double>, int>(state, [](auto& server){
    server->reg(&InterfaceT::get_rand_nums, [](int size) {
      std::vector<double> ret(size);
      std::iota(ret.begin(), ret.end(), 0.);
      return ret;
    });
    },
    [](auto& client){
      return client->call(&InterfaceT::get_rand_nums,
                          int(DATASIZE / sizeof(double)));
    });
}

static void BM_NoNet_NoSerial_send_rcv_large_data(benchmark::State& state) {
  typedef benchmark_interface<IdentitySerializer<std::vector<unsigned char>, std::vector<unsigned char>>> InterfaceT;
  BM_NoNet_NoSerial_Template<std::vector<unsigned char>, std::vector<unsigned char>>(state, [](auto& server){
        server->reg(&InterfaceT::send_rcv_large_data,
                    [](std::vector<unsigned char> l_data) {
                      return l_data;
                    });
      },
      [](auto& client){
        return client->call(&InterfaceT::send_rcv_large_data,
                            std::vector<unsigned char>(DATASIZE / sizeof(unsigned char)));
      });
}

template<int funcNum>
static void BM_NoNetwork_NoSerializer(benchmark::State& state) {
  if constexpr (funcNum == 0) {
    BM_NoNet_NoSerial_say_hello(state);
  }
  else if constexpr (funcNum == 1) {
    BM_NoNet_NoSerial_average(state);
  }
  else if constexpr (funcNum == 2) {
    BM_NoNet_NoSerial_get_rand_nums(state);
  }
  else {
    BM_NoNet_NoSerial_send_rcv_large_data(state);
  }
}

template<int funcNum>
static void BM_Latency_NoNetwork_NoSerializer(benchmark::State& state) {
  if constexpr (funcNum == 0) {
    typedef benchmark_interface<IdentitySerializer<std::string, std::string>> InterfaceT;
    BM_NoNet_NoSerial_Latency_Template<std::string, std::string>(state, [](auto& server){
          server->reg(&InterfaceT::say_hello, [](std::string ms) {
            return std::string("hello " + ms);
          });
        },
        [](auto& client){
          return client->call(&InterfaceT::say_hello,
                              std::string("peter"));
        },
        "latencies_NoNetwork_NoSerializer_<0>.txt");
  }
  else if constexpr (funcNum == 1) {
    typedef benchmark_interface<IdentitySerializer<double, std::vector<double>>> InterfaceT;
    BM_NoNet_NoSerial_Latency_Template<double, std::vector<double>>(state, [](auto& server){
          server->reg(&InterfaceT::average,
                      [](const std::vector<double>& vec) {
                        double avg = 0.;
                        for (double element : vec) avg += element / vec.size();
                        return avg;
                      });
        },
        [](auto& client){
          return client->call(&InterfaceT::average,
                              std::vector<double>(1024 * 64 / sizeof(double)));
        },
        "latencies_NoNetwork_NoSerializer_<1>.txt");
  }
  else if constexpr (funcNum == 2) {
    typedef benchmark_interface<IdentitySerializer<std::vector<double>, int>> InterfaceT;
    BM_NoNet_NoSerial_Latency_Template<std::vector<double>, int>(state, [](auto& server){
          server->reg(&InterfaceT::get_rand_nums, [](int size) {
            std::vector<double> ret(size);
            std::iota(ret.begin(), ret.end(), 0.);
            return ret;
          });
        },
        [](auto& client){
          return client->call(&InterfaceT::get_rand_nums,
                              int(DATASIZE / sizeof(double)));
        },
        "latencies_NoNetwork_NoSerializer_<2>.txt");
  }
  else {
    typedef benchmark_interface<IdentitySerializer<std::vector<unsigned char>, std::vector<unsigned char>>> InterfaceT;
    BM_NoNet_NoSerial_Latency_Template<std::vector<unsigned char>, std::vector<unsigned char>>(state, [](auto& server){
          server->reg(&InterfaceT::send_rcv_large_data,
                      [](std::vector<unsigned char> l_data) {
                        return l_data;
                      });
        },
        [](auto& client){
          return client->call(&InterfaceT::send_rcv_large_data,
                              std::vector<unsigned char>(DATASIZE / sizeof(unsigned char)));
        },
        "latencies_NoNetwork_NoSerializer_<3>.txt");
  }
}