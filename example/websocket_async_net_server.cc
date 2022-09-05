#include "crpc/rpc_async_websocket_net_server.h"

#include "benchmark_interface.h"

int main(int argc, char* argv[]) {
    int thread_count = std::atoi(argv[1]);
    boost::asio::io_service ios(thread_count);

    auto server = rpc_async_websocket_net_server<benchmark_interface>{ios, "0.0.0.0", "2000"};
    register_benchmark_interface(server);

    server.run(thread_count);

    return 0;
}
