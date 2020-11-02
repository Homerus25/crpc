#include <iostream>

#include "crpc/rpc_async_thread_websocket_client.h"
#include "example_interface.h"

int main(int argc, char* argv[]) {
    rpc_async_thread_websocket_client<example_interface> client{std::string("127.0.0.1"), 2000u};

    //auto res = client.call(&example_interface::add_, 5, 2);
    //std::cout << res() << "\n";

    /*
    std::cout << client.call(&example_interface::add_, 5, 2)() << "\n";
    client.call(&example_interface::hello_world_)();
    client.call(&example_interface::inc_count_, 5)();
    std::cout << client.call(&example_interface::get_count_)() << "\n";
     */
    example_run(client);

    return 0;
}