#include <iostream>

#include "crpc/rpc_async_websocket_net_client.h"
#include "benchmark_interface.h"
#include "benchmark.h"

int main(int argc, char* argv[]) {
  auto parameter = parse_benchmark_parameters(argc, argv);
  if(!parameter.has_value()) {
    return 0;
  }

  benchmark bench(parameter->clientCount, [&]{
    rpc_async_websocket_client<benchmark_interface> client{
        std::string("127.0.0.1"), "2000",
        get_benchmark_function(client, bench, *parameter)};
    bench.save_time(client.get_times());
  });

  return 0;
}
