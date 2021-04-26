#include <iostream>

#include "crpc/rpc_async_thread_websocket_client.h"
#include "benchmark_interface.h"

int main(int argc, char* argv[]) {

    rpc_async_thread_websocket_client<benchmark_interface> client{std::string("127.0.0.1"), 2000u};

    if (argc == 3) {
      int iterations = std::atoi(argv[2]);
      if(iterations == 0) {
        std::cout << "invalid iteration count!\n";
        return 1;
      }
      if (*argv[1] == '0') {
        benchmark_run(client, iterations);
      }
      else if (*argv[1] == '1') {
        benchmark_run_say_hello(client, iterations);
      }
    }
    else {
      std::cout << "must be 2 arguments!\n";
      return 1;
    }

    return 0;
}
