#define CRPC_LOG TRUE

#include "crpc/http2/http2_server.h"
#include "../../src/benchmark/benchmark_interface.h"
#include "crpc/serialization/zpp_bits_serialization.h"

int main(int argc, char* argv[]) {
  typedef http2_server<benchmark_interface<ZppBitsSerializer>, ZppBitsSerializer> ServerType;
  ServerType server{};
  register_benchmark_interface<ServerType, ZppBitsSerializer>(server);

  auto threads_count = argc == 2 ? std::atoi(argv[1]) : 1;
  server.run_blocked(threads_count);
  //server.block();
}