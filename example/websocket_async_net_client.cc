#include <iostream>

#include "crpc/rpc_async_websocket_net_client.h"
//#include "example_interface.h"
#include "benchmark_interface.h"

int main(int argc, char* argv[]) {

  rpc_async_websocket_client<benchmark_interface> client {
    std::string("127.0.0.1"), "2000", get_benchmark_function(client, argc, argv)};

  return 0;
}
