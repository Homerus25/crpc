#include <benchmark/benchmark.h>
#include "benchmark_interface.h"
#include "bench.h"

#include "crpc/http/http_ws_server.h"
#include "crpc/http/rpc_http_client.h"
#include "crpc/http2/http2_server.h"
#include "crpc/http2/http2_client.h"
#include "crpc/http/rpc_ws_client.h"
#include "crpc/mqtt/rpc_mqtt_server.h"
#include "crpc/mqtt/rpc_mqtt_transport.h"

#include "crpc/serialization/cista.h"
#include "crpc/serialization/glaze_binary.h"
#include "crpc/serialization/glaze_json.h"
#include "crpc/serialization/tuple_serialization.h"
#include "crpc/serialization/zpp_bits_serialization.h"

#include "no_serializer_bench.h"
#include "benchmark_operations.h"

typedef ZppBitsSerializer DefaultSerializer;

template <int funcNum>
static void BM_NoNetwork_Cista(benchmark::State& state) {
  FloodBench<no_network_server, no_network_client, benchmark_interface, CistaSerialzer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_NoNetwork_Tuple(benchmark::State& state) {
  FloodBench<no_network_server, no_network_client, benchmark_interface,
             TupleSerializer> bench(state.range(0), state.range(1));
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
  FloodBench<http_ws_server, rpc_http_client, benchmark_interface, DefaultSerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_HTTP2(benchmark::State& state) {
  FloodBench<http2_server, rpc_http2_client, benchmark_interface, DefaultSerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_WS(benchmark::State& state) {
  FloodBench<http_ws_server, rpc_ws_client, benchmark_interface, DefaultSerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_MQTT(auto& state) {
  FloodBench<rpc_mqtt_server, rpc_mqtt_client, benchmark_interface, DefaultSerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_CUSTOM_AVERAGE(benchmark::State& state) {
  FloodBench<rpc_mqtt_server, rpc_mqtt_client, benchmark_interface, GlazeBinarySerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
}

template <int funcNum>
static void BM_Latency_CUSTOM_AVERAGE(benchmark::State& state) {
  LatencyBench<rpc_mqtt_server, rpc_mqtt_client, benchmark_interface, GlazeBinarySerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_custom_<" + std::to_string(funcNum) + ">.txt");
}


template <int funcNum>
static void BM_Latency_NoNetwork_Cista(benchmark::State& state) {
  LatencyBench<no_network_server, no_network_client, benchmark_interface, CistaSerialzer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_NoNetwork_Cista_<" + std::to_string(funcNum) + ">.txt");
}

template <int funcNum>
static void BM_Latency_NoNetwork_Tuple(benchmark::State& state) {
  LatencyBench<no_network_server, no_network_client, benchmark_interface,
               TupleSerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_NoNetwork_Tuple_<" + std::to_string(funcNum) + ">.txt");
}

template <int funcNum>
static void BM_Latency_NoNetwork_GlazeJSON(benchmark::State& state) {
  LatencyBench<no_network_server, no_network_client, benchmark_interface, GlazeJSONSerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_NoNetwork_JSON_<" + std::to_string(funcNum) + ">.txt");
}

template <int funcNum>
static void BM_Latency_NoNetwork_GlazeBinary(benchmark::State& state) {
  LatencyBench<no_network_server, no_network_client, benchmark_interface, GlazeBinarySerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_NoNetwork_Crusher_<" + std::to_string(funcNum) + ">.txt");
}

template <int funcNum>
static void BM_Latency_NoNetwork_ZppBits(benchmark::State& state) {
  LatencyBench<no_network_server, no_network_client, benchmark_interface, ZppBitsSerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_NoNetwork_zppbits_<" + std::to_string(funcNum) + ">.txt");
}



template <int funcNum>
static void BM_Latency_HTTP(benchmark::State& state) {
  LatencyBench<http_ws_server, rpc_http_client, benchmark_interface, DefaultSerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_HTTP_<" + std::to_string(funcNum) + ">.txt");
}

template <int funcNum>
static void BM_Latency_HTTP2(benchmark::State& state) {
  LatencyBench<http2_server, rpc_http2_client, benchmark_interface, DefaultSerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_HTTP2_<" + std::to_string(funcNum) + ">.txt");
}

template <int funcNum>
static void BM_Latency_WS(benchmark::State& state) {
  LatencyBench<http_ws_server, rpc_ws_client, benchmark_interface, DefaultSerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_WS_<" + std::to_string(funcNum) + ">.txt");
}

template <int funcNum>
static void BM_Latency_MQTT(benchmark::State& state) {
  LatencyBench<rpc_mqtt_server, rpc_mqtt_client, benchmark_interface,  DefaultSerializer> bench(state.range(0), state.range(1));
  bench.run<funcNum>(state);
  bench.writeLatencies("latencies_MQTT_<" + std::to_string(funcNum) + ">.txt");
}



static void FloodBenchArguments(benchmark::internal::Benchmark* b) {
  //b->UseRealTime()->ArgsProduct({benchmark::CreateRange(1, 64, 2), benchmark::CreateRange(1, 64, 2)})->Unit(benchmark::kMillisecond);//->Repetitions(3);
  b->UseRealTime()->ArgPair(4, 4)->Unit(benchmark::kMillisecond)->Repetitions(3);
}

static void LatencyBenchArguments(benchmark::internal::Benchmark* b) {
  b->UseRealTime()->Iterations(1)->ArgPair(1, 1)->Unit(benchmark::kMillisecond);
}

//Flood Benchs
//Only Serializers
BENCHMARK(BM_NoNetwork_NoSerializer<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_NoSerializer<1>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_NoSerializer<2>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_NoSerializer<3>)->Apply(FloodBenchArguments);

BENCHMARK(BM_NoNetwork_Cista<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_Cista<1>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_Cista<2>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_Cista<3>)->Apply(FloodBenchArguments);

BENCHMARK(BM_NoNetwork_GlazeJSON<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_GlazeJSON<1>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_GlazeJSON<2>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_GlazeJSON<3>)->Apply(FloodBenchArguments);

BENCHMARK(BM_NoNetwork_GlazeBinary<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_GlazeBinary<1>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_GlazeBinary<2>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_GlazeBinary<3>)->Apply(FloodBenchArguments);

BENCHMARK(BM_NoNetwork_ZppBits<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_ZppBits<1>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_ZppBits<2>)->Apply(FloodBenchArguments);
BENCHMARK(BM_NoNetwork_ZppBits<3>)->Apply(FloodBenchArguments);


BENCHMARK(BM_CUSTOM_AVERAGE<1>)->Apply(FloodBenchArguments);
BENCHMARK(BM_CUSTOM_AVERAGE<3>)->Apply(FloodBenchArguments);

//Network Protokolls
BENCHMARK(BM_HTTP<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_HTTP<1>)->Apply(FloodBenchArguments);
BENCHMARK(BM_HTTP<2>)->Apply(FloodBenchArguments);
BENCHMARK(BM_HTTP<3>)->Apply(FloodBenchArguments);

BENCHMARK(BM_HTTP2<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_HTTP2<1>)->Apply(FloodBenchArguments);
BENCHMARK(BM_HTTP2<2>)->Apply(FloodBenchArguments);
BENCHMARK(BM_HTTP2<3>)->Apply(FloodBenchArguments);

BENCHMARK(BM_WS<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_WS<1>)->Apply(FloodBenchArguments);
BENCHMARK(BM_WS<2>)->Apply(FloodBenchArguments);
BENCHMARK(BM_WS<3>)->Apply(FloodBenchArguments);

BENCHMARK(BM_MQTT<0>)->Apply(FloodBenchArguments);
BENCHMARK(BM_MQTT<1>)->Apply(FloodBenchArguments);
BENCHMARK(BM_MQTT<2>)->Apply(FloodBenchArguments);
BENCHMARK(BM_MQTT<3>)->Apply(FloodBenchArguments);

BENCHMARK(BM_Latency_CUSTOM_AVERAGE<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_CUSTOM_AVERAGE<1>)->Apply(LatencyBenchArguments);



//Latency Tests
//Only Serializers
BENCHMARK(BM_Latency_NoNetwork_NoSerializer<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_NoSerializer<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_NoSerializer<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_NoSerializer<3>)->Apply(LatencyBenchArguments);

BENCHMARK(BM_Latency_NoNetwork_Cista<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_Cista<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_Cista<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_Cista<3>)->Apply(LatencyBenchArguments);

BENCHMARK(BM_Latency_NoNetwork_GlazeJSON<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_GlazeJSON<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_GlazeJSON<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_GlazeJSON<3>)->Apply(LatencyBenchArguments);

BENCHMARK(BM_Latency_NoNetwork_GlazeBinary<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_GlazeBinary<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_GlazeBinary<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_GlazeBinary<3>)->Apply(LatencyBenchArguments);

BENCHMARK(BM_Latency_NoNetwork_ZppBits<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_ZppBits<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_ZppBits<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_NoNetwork_ZppBits<3>)->Apply(LatencyBenchArguments);


//Network Protokolls
BENCHMARK(BM_Latency_HTTP<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_HTTP<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_HTTP<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_HTTP<3>)->Apply(LatencyBenchArguments);

BENCHMARK(BM_Latency_HTTP2<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_HTTP2<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_HTTP2<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_HTTP2<3>)->Apply(LatencyBenchArguments);

BENCHMARK(BM_Latency_WS<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_WS<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_WS<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_WS<3>)->Apply(LatencyBenchArguments);

BENCHMARK(BM_Latency_MQTT<0>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_MQTT<1>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_MQTT<2>)->Apply(LatencyBenchArguments);
BENCHMARK(BM_Latency_MQTT<3>)->Apply(LatencyBenchArguments);

BENCHMARK_MAIN();