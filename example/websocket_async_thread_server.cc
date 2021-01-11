#include "crpc/rpc_async_thread_websocket_server.h"

//#include "example_interface.h"
#include "benchmark_interface.h"

int main(int argc, char* argv[]) {

    //auto server = rpc_async_thread_websocket_server<example_interface>{2000};
    //register_example_interface(server);

    auto server = rpc_async_thread_websocket_server<benchmark_interface>{2000};
    register_benchmark_interface(server);

    server.run();

    return 0;
}
