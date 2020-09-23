#include <iostream>

#include "crpc/rpc_async_websocket_client.h"
#include "example_interface.h"

int main(int argc, char* argv[]) {
    //boost::asio::ssl::context ssl_context{boost::asio::ssl::context::sslv23};
    //boost::asio::ssl::context ssl_context{boost::asio::ssl::context::no_sslv2};

    //boost::asio::io_service ios;
    //rpc_async_websocket_client<example_interface> client{ios, std::string("127.0.0.1"), "2000"};//, ssl_context};


    //auto res = client.call(&example_interface::add_, 5, 2);
    //std::cout << res() << "\n";

    rpc_async_websocket_client<example_interface> client{std::string("127.0.0.1"), "2000",
    [&] () {
        std::cout << client.call(&example_interface::add_, 5, 2)() << "\n";
        client.call(&example_interface::hello_world_)();
        client.call(&example_interface::inc_count_, 5)();
        std::cout << client.call(&example_interface::get_count_)() << "\n";
    }
    };

    //ios.run();

    return 0;
}
