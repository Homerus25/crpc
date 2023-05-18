#include "crpc/http/http_ws_server.h"
#include "../benchmark_interface.h"

int main(int argc, char* argv[]) {
  boost::asio::io_context ioc;
  http_ws_server<benchmark_interface> server{ioc};
  register_benchmark_interface(server);
  ioc.run();
}