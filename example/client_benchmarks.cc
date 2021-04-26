#include "example_interface.h"

#include "crpc/rpc_async_thread_websocket_client.h"
#include "crpc/rpc_async_websocket_net_client.h"

#include <iostream>

void bench_ctx()
{
    rpc_async_websocket_client<example_interface> client{
      std::string("127.0.0.1"), "2000",
      [&] () {
          example_run(client);
      }
    };
}

void bench_single_tread()
{
    rpc_async_thread_websocket_client <example_interface> client{std::string("127.0.0.1"), 2000u};
    example_run(client);
}

int main(int argc, char* argv[]) {
    bench_ctx();
    //benchAsio();
    bench_single_tread();

    return 0;
}
