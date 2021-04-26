#include "crpc/rpc_async_thread_websocket_server.h"

#include "benchmark_interface.h"

int main(int argc, char* argv[]) {

    auto server = rpc_async_thread_websocket_server<benchmark_interface>{2000};
    register_benchmark_interface(server);

    server.run();

    return 0;
}
