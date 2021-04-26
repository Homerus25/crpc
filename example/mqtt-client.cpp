#include "benchmark_interface.h"

#include "crpc/rpc_mqtt_transport.h"
#include "crpc/rpc_mqtt_transport.h"

int main(int argc, char* argv[]) {

  boost::asio::io_context ioc;

  if (argc == 3) {
    int iterations = std::atoi(argv[2]);
    if(iterations == 0) {
      std::cout << "invalid iteration count!\n";
      return 1;
    }
    if (*argv[1] == '0') {
      rpc_mqtt_client<benchmark_interface> client{
          ioc, std::string("127.0.0.1"), std::uint16_t(2000)};
          benchmark_run(client, iterations);
    }
    else if (*argv[1] == '1') {
      boost::asio::post(ioc, [&]() {
        rpc_mqtt_client<benchmark_interface> client{
            ioc, std::string("localhost"), std::uint16_t(2000)};
        benchmark_run_say_hello(client, iterations); //std::exit(0);
      });
      }
      std::thread t1([&](){ ioc.run(); });

      ioc.run();
      t1.join();
  }
  else {
    std::cout << "must be 2 arguments!\n";
    return 1;
  }

  return 0;
}
