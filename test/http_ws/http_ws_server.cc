#include "crpc/http/http_ws_server.h"
#include "../../src/benchmark/benchmark_interface.h"
#include "crpc/serialization/zpp_bits_serialization.h"

int main(int argc, char* argv[]) {
  typedef http_ws_server<benchmark_interface<ZppBitsSerializer>, ZppBitsSerializer> ServerType;
  ServerType server{};
  register_benchmark_interface<ServerType, ZppBitsSerializer>(server);

  auto threads_count = argc == 2 ? std::atoi(argv[1]) : 1;
  server.run(threads_count - 1);
  server.block();
}