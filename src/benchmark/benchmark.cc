#include <benchmark/benchmark.h>
#include "benchmark_interface.h"
#include "bench.h"

#include "crpc/no_network/no_network_client.h"
#include "crpc/no_network/no_network_server.h"
#include "crpc/http/http_ws_server.h"
#include "crpc/http/rpc_http_client.h"
#include "crpc/http/rpc_ws_client.h"
#include "crpc/mqtt/rpc_mqtt_server.h"
#include "crpc/mqtt/rpc_mqtt_transport.h"

#include "crpc/serialization/cista.h"
#include "crpc/serialization/no_serialization.h"
#include "crpc/serialization/glaze_json.h"
#include "crpc/serialization/glaze_binary.h"
#include "crpc/serialization/zpp_bits_serialization.h"

template <int funcNum>
static void BM_NoNetwork(benchmark::State& state) {
  FloodBench<no_network_server, no_network_client, benchmark_interface, CistaSerialzer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_NoNetwork_NoSerial(benchmark::State& state) {
  FloodBench<no_network_server, no_network_client, benchmark_interface,
             NoSerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_NoNetwork_GlazeJSON(benchmark::State& state) {
  FloodBench<no_network_server, no_network_client, benchmark_interface,
             GlazeJSONSerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_NoNetwork_GlazeBinary(benchmark::State& state) {
  FloodBench<no_network_server, no_network_client, benchmark_interface,
             GlazeBinarySerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_NoNetwork_ZppBits(benchmark::State& state) {
  FloodBench<no_network_server, no_network_client, benchmark_interface,
             ZppBitsSerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}


template <int funcNum>
static void BM_HTTP(benchmark::State& state) {
  FloodBench<http_ws_server, rpc_http_client, benchmark_interface, CistaSerialzer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_WS(benchmark::State& state) {
  FloodBench<http_ws_server, rpc_ws_client, benchmark_interface, CistaSerialzer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_MQTT(auto& state) {
  FloodBench<rpc_mqtt_server, rpc_mqtt_client, benchmark_interface, CistaSerialzer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}


template <int funcNum>
static void BM_Latency_NoNetwork(benchmark::State& state) {
  LatencyBench<no_network_server, no_network_client, benchmark_interface, CistaSerialzer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_NoNetwork_<" + std::to_string(funcNum) + ">.txt");
}

template <int funcNum>
static void BM_Latency_HTTP(benchmark::State& state) {
  LatencyBench<http_ws_server, rpc_http_client, benchmark_interface, CistaSerialzer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_HTTP_<" + std::to_string(funcNum) + ">.txt");
}

template <int funcNum>
static void BM_Latency_WS(benchmark::State& state) {
  LatencyBench<http_ws_server, rpc_ws_client, benchmark_interface, CistaSerialzer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_WS_<" + std::to_string(funcNum) + ">.txt");
}

template <int funcNum>
static void BM_Latency_MQTT(benchmark::State& state) {
  LatencyBench<rpc_mqtt_server, rpc_mqtt_client, benchmark_interface,  CistaSerialzer> bench(state.range(0), state.range(1));
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


BENCHMARK(BM_NoNetwork_NoSerial<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_GlazeJSON<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_GlazeBinary<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_ZppBits<0>)->Apply(FloodBenchArguments);


BENCHMARK(BM_Latency_NoNetwork<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork<3>)->Apply(LatencyBenchArguments);

BENCHMARK(BM_Latency_HTTP<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_HTTP<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_HTTP<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_HTTP<3>)->Apply(LatencyBenchArguments);

BENCHMARK(BM_Latency_WS<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_WS<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_WS<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_WS<3>)->Apply(LatencyBenchArguments);

BENCHMARK(BM_Latency_MQTT<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_MQTT<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_MQTT<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_MQTT<3>)->Apply(LatencyBenchArguments);


BENCHMARK_MAIN();