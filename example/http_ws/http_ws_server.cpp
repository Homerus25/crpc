#include "crpc/http/http_ws_server.h"
#include "../benchmark_interface.h"

int main(int argc, char* argv[]) {
  http_ws_server<benchmark_interface> server{};
  register_benchmark_interface(server);
  server.run();
}