#include <iostream>

#include "crpc/rpc_async_websocket_net_client.h"
//#include "example_interface.h"
#include "benchmark_interface.h"
//#include <barrier>
//#include <latch>
#include "benchmark.h"

int main(int argc, char* argv[]) {
  /*
  rpc_async_websocket_client<benchmark_interface> client {
    std::string("127.0.0.1"), "2000", get_benchmark_function(client, argc, argv)};
    */


  auto parameter = parse_benchmark_parameters(argc, argv);
  if(!parameter.has_value()) {
    return 0;
  }

  benchmark bench(parameter->clientCount, [&]{
    rpc_async_websocket_client<benchmark_interface> client{
        std::string("127.0.0.1"), "2000",
        get_benchmark_function(client, bench, *parameter)};
  });

/*
std::vector<std::thread> trs;
trs.reserve(client_count);
std::mutex save_mutex;
std::vector<int> times;
times.reserve(client_count);
//std::barrier sync_point(client_count, []{ std::cout << "b\n"; });
std::latch sync(5);
int counter = 0;
std::cout << "start" << std::endl;
while (counter < client_count) {
  trs.push_back(std::thread([&]{
    rpc_async_websocket_client<benchmark_interface> client{
        std::string("127.0.0.1"), "2000",
        get_benchmark_function(client, save_mutex, times, argc, argv)};
    }
  ));
  ++counter;
}
std::cout << "all started" << std::endl;

for(auto &tr : trs) {
    tr.join();
}
std::cout << "finished!" << std::endl;
*/

  return 0;
}
