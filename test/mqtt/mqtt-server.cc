#include "../../src/benchmark/benchmark_interface.h"
#include "crpc/mqtt/rpc_mqtt_server.h"
#include "crpc/serialization/cista.h"

int main(int argc, char* argv[]) {
  auto server = rpc_mqtt_server<benchmark_interface, CistaSerialzer>(2000);
  register_benchmark_interface(server);
  server.block();
  server.stop();
}