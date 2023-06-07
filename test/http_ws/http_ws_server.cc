#include "crpc/http/http_ws_server.h"
#include "../../src/benchmark/benchmark_interface.h"

int main(int argc, char* argv[]) {
  http_ws_server<benchmark_interface> server{};
  register_benchmark_interface(server);

  auto threads_count = argc == 2 ? std::atoi(argv[1]) : 1;
  server.run(threads_count - 1);
  server.block();
}