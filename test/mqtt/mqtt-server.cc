#include "../../src/benchmark/benchmark_interface.h"
#include "crpc/mqtt/rpc_mqtt_server.h"
#include "crpc/serialization/zpp_bits_serialization.h"

int main(int argc, char* argv[]) {
  typedef rpc_mqtt_server<benchmark_interface<ZppBitsSerializer>, ZppBitsSerializer> ServerType;
  auto server = ServerType(2000);
  register_benchmark_interface<ServerType, ZppBitsSerializer>(server);
  server.block();
  server.stop();
}