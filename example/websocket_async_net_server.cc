#include "crpc/rpc_async_websocket_net_server.h"

#include "benchmark_interface.h"

int main(int argc, char* argv[]) {

    boost::asio::io_service ios(8);

    auto server = rpc_async_websocket_net_server<benchmark_interface>{ios, "0.0.0.0", "2000"};
    register_benchmark_interface(server);

    server.run();
    ios.run();

    return 0;
}
