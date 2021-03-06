#include "benchmark_interface.h"
#include "crpc/rpc_mqtt_server.h"

int main(int argc, char* argv[]) {

  auto server = rpc_mqtt_server<benchmark_interface>(2000);
  register_benchmark_interface(server);
  server.run();
}

