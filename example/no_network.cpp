#include "crpc/no_network/no_network_client.h"
#include "crpc/no_network/no_network_server.h"
#include "benchmark.h"
#include "benchmark_interface.h"

int main(int argc, char* argv[]) {
  auto parameter = parse_benchmark_parameters(argc, argv);
  if(!parameter.has_value()) {
    return 0;
  }

  auto server = no_network_server<benchmark_interface>();
  register_benchmark_interface(server);

  int thread_count = std::atoi(argv[argc - 1]);
  auto st = std::thread([&](){ server.run(thread_count); });

  benchmark bench(parameter->clientCount, [&] {
    no_network_client<benchmark_interface> client{ [&](const std::vector<uint8_t> message, auto rcv) { server.receive(message, rcv); } };
    get_benchmark_function(client, bench, *parameter)();
    bench.save_time(client.get_times());
  });

  server.kill();
  st.join();
}
