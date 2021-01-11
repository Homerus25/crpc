#include <iostream>

#include "crpc/rpc_async_websocket_client.h"
//#include "example_interface.h"
#include "benchmark_interface.h"

int main(int argc, char* argv[]) {
  /*
    rpc_async_websocket_client<example_interface>
    client{std::string("127.0.0.1"), "2000",
    [&] () {
        std::cout << client.call(&example_interface::add_, 5, 2)() << "\n";
        client.call(&example_interface::hello_world_)();
        client.call(&example_interface::inc_count_, 5)();
        std::cout << client.call(&example_interface::get_count_)() << "\n";
    }
    };
    */

  /*
  rpc_async_websocket_client<example_interface> client{std::string("127.0.0.1"),
  "2000",
     [&] () {
        example_run(client);
      }
  };
  */

  if (argc == 3) {
    int iterations = std::atoi(argv[2]);
    if(iterations == 0) {
      std::cout << "invalid iteration count!\n";
      return 1;
    }
    if (*argv[1] == '0') {
      rpc_async_websocket_client<benchmark_interface> client{
          std::string("127.0.0.1"), "2000",
          [&]() { benchmark_run(client, iterations); }};
    }
    else if (*argv[1] == '1') {
      rpc_async_websocket_client<benchmark_interface> client{
          std::string("127.0.0.1"), "2000",
          [&]() { benchmark_run_say_hello(client, iterations); }};
    }
  }
  else {
    std::cout << "must be 2 arguments!\n";
    return 1;
  }

  return 0;
}
