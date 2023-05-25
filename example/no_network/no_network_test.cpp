#include "../../src/benchmark/benchmark_interface.h"
#include "crpc/no_network/no_network_client.h"
#include "crpc/no_network/no_network_server.h"

#include <iostream>

int main(int argc, char* argv[]) {

  auto server = no_network_server<benchmark_interface>();
  register_benchmark_interface(server);

  auto st = std::thread([&](){ server.run(4); });

  no_network_client<benchmark_interface> client{ [&](const std::vector<uint8_t> message, auto rcv) { server.receive(message, rcv); } };
  client.run(4);


  auto resObj = client.call(&benchmark_interface::say_hello, data::string("Peter"));
  auto res = resObj();
  std::cout << res;

  std::cout << client.call(&benchmark_interface::average, data::vector<double>({10.0, 100.0, 20.0}))();

  client.stop();
  server.stop();
  st.join();
}