#include "crpc/http/http_ws_server.h"
#include "../benchmark_interface.h"

int main(int argc, char* argv[]) {
  boost::asio::io_context ioc;
  http_ws_server<benchmark_interface> server{ioc};
  register_benchmark_interface(server);

  auto threads_count = argc == 2 ? std::atoi(argv[1]) : 1;
  std::vector<std::thread> thread_pool;
  for(int i=0; i<threads_count-1; ++i)
    thread_pool.emplace_back([&](){ ioc.run(); });

  ioc.run();
  for (auto& thread : thread_pool)
    thread.join();
}