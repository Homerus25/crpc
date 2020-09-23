#include "crpc/rpc_async_websocket_server.h"

#include "example_interface.h"

#include <iostream>
#include <chrono>

int main(int argc, char* argv[]) {

    boost::asio::io_service ios;
    auto server = rpc_async_websocket_server<example_interface>{ios, "0.0.0.0", "2000"};

    int count = 0;

    server.reg(&example_interface::add_, [](int a, int b) { return a + b; });
    server.reg(&example_interface::hello_world_, [&]() { std::cout << "hello world\n"; });
    server.reg(&example_interface::inc_count_, [&](int i) { return count += i; });
    server.reg(&example_interface::get_count_, [&]() { return count; });

    server.run();
    ios.run();

    return 0;
}
