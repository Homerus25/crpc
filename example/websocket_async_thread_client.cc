#include <iostream>

#include "crpc/rpc_async_thread_websocket_client.h"
//#include "example_interface.h"
#include "benchmark_interface.h"

int main(int argc, char* argv[]) {
    //rpc_async_thread_websocket_client<example_interface> client{std::string("127.0.0.1"), 2000u};

    //auto res = client.call(&example_interface::add_, 5, 2);
    //std::cout << res() << "\n";

    /*
    std::cout << client.call(&example_interface::add_, 5, 2)() << "\n";
    client.call(&example_interface::hello_world_)();
    client.call(&example_interface::inc_count_, 5)();
    std::cout << client.call(&example_interface::get_count_)() << "\n";
     */
    //example_run(client);

    rpc_async_thread_websocket_client<benchmark_interface> client{std::string("127.0.0.1"), 2000u};
    //benchmark_run(client, 1000);

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
