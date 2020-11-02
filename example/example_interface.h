#pragma once

#include "crpc/rpc_client.h"

#include <iostream>

struct example_interface {
    fn<int, int, int> add_;
    fn<void> hello_world_;
    fn<void, int> inc_count_;
    fn<int> get_count_;
};

template<typename Client>
void example_run(Client& client)
{
  {
    char unused;
    std::cin >> unused;
  }
  auto start = std::chrono::high_resolution_clock::now();
  for(auto i = 0; i < 1000; ++i) {
    //std::cout << client.call(&example_interface::add_, 5, 2)() << "\n";
    client.call(&example_interface::add_, 5, 2)();
    //client.call(&example_interface::hello_world_)();
    client.call(&example_interface::inc_count_, 5)();
    client.call(&example_interface::get_count_)();
    //std::cout << client.call(&example_interface::get_count_)() << "\n";
  }
  auto end = std::chrono::high_resolution_clock::now();
  //std::cout << "time: " << duration_cast<std::chrono::milliseconds>(end - start).count() << "\n";
  std::cout << duration_cast<std::chrono::milliseconds>(end - start).count();// << "\n";
}

template<typename Server>
void register_example_interface(Server &server)
{
  static int count = 0;

  server.reg(&example_interface::add_, [](int a, int b) { return a + b; });
  server.reg(&example_interface::hello_world_,
             [&]() { std::cout << "hello world\n"; });
  server.reg(&example_interface::inc_count_, [&](int i) { return count += i; });
  server.reg(&example_interface::get_count_, [&]() { return count; });
}
