#include "benchmark_interface.h"

#include "crpc/rpc_mqtt_transport.h"
#include "crpc/rpc_mqtt_transport.h"

#include "benchmark.h"

int main(int argc, char* argv[]) {

  auto parameter = parse_benchmark_parameters(argc, argv);
  if(!parameter.has_value()) {
    return 0;
  }

  benchmark bench(parameter->clientCount, [&] {
    boost::asio::io_context ioc;

    boost::asio::post(ioc, [&]() {
      rpc_mqtt_client<benchmark_interface> client{
          ioc, std::string("localhost"), std::uint16_t(2000)};
      get_benchmark_function(client, bench, *parameter)();
    });

    std::thread t1([&](){ ioc.run(); });

    ioc.run();
    t1.join();
  }
 );

  return 0;
}
