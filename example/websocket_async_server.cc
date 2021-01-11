#include "crpc/rpc_async_websocket_server.h"

//#include "example_interface.h"
#include "benchmark_interface.h"

#include <iostream>
#include <chrono>

int main(int argc, char* argv[]) {

    boost::asio::io_service ios(8);

    //auto server = rpc_async_websocket_server<example_interface>{ios, "0.0.0.0", "2000"};
    //register_example_interface(server);

    auto server = rpc_async_websocket_server<benchmark_interface>{ios, "0.0.0.0", "2000"};
    register_benchmark_interface(server);

    server.run();
    ios.run();

    return 0;
}
