#include "doctest/doctest.h"

#include <sstream>

#include "crpc/no_network/no_network_client.h"
#include "crpc/rpc_server.h"

TEST_CASE("no network test") {
    struct interface {
        fn<int, int, int> add_;
        fn<void> hello_world_;
        fn<void, int> inc_count_;
        fn<int> get_count_;
    };

    int count = 0;
    auto server = no_network_server<interface>();
    std::stringstream out;
    server.reg(&interface::add_, [](int a, int b) { return a + b; });
    server.reg(&interface::hello_world_, [&]() { out << "hello world"; });
    server.reg(&interface::inc_count_, [&](int i) { return count += i; });
    server.reg(&interface::get_count_, [&]() { return count; });
    auto st = std::thread([&](){ server.run(1); });

    no_network_client<interface> client{ [&](const std::vector<uint8_t> message, auto rcv) { server.receive(message, rcv); } };
    client.run(1);
    CHECK(client.call(&interface::add_, 1, 2)() == 3);
    CHECK_NOTHROW(client.call(&interface::hello_world_)());
    CHECK(out.str() == "hello world");
    CHECK_NOTHROW(client.call(&interface::inc_count_, 5)());
    CHECK(client.call(&interface::get_count_)() == 5);
    client.stop();
    server.stop();
    st.join();
}
