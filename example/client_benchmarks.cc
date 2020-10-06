#include "example_interface.h"

#include "crpc/rpc_async_websocket_client.h"
#include "crpc/rpc_async_thread_websocket_client.h"

#include <iostream>

template<typename Client>
void run(Client& client)
{
    auto start = std::chrono::high_resolution_clock::now();
    for(auto i = 0; i < 10000; ++i) {
        //std::cout << client.call(&example_interface::add_, 5, 2)() << "\n";
        client.call(&example_interface::add_, 5, 2)();
        //client.call(&example_interface::hello_world_)();
        client.call(&example_interface::inc_count_, 5)();
        client.call(&example_interface::get_count_)();
        //std::cout << client.call(&example_interface::get_count_)() << "\n";
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "time: " << duration_cast<std::chrono::milliseconds>(end - start).count() << "\n";
}

void benchCtx()
{
    rpc_async_websocket_client<example_interface> client{std::string("127.0.0.1"), "2000",
                                                         [&] () {
                                                            run(client);
                                                         }
    };
}

void benchSingleTread()
{
    rpc_async_thread_websocket_client <example_interface> client{std::string("127.0.0.1"), 2000u};
    run(client);
}

int main(int argc, char* argv[]) {
    benchCtx();
    //benchAsio();
    benchSingleTread();

    return 0;
}
