#include "../benchmark_interface.h"
#include "crpc/http/rpc_ws_client.h"

int main(int argc, char* argv[]) {
  rpc_ws_client<benchmark_interface> client{std::string("127.0.0.1"), 9000u};
  auto resp = client.call(&benchmark_interface::say_hello, data::string("peter"));
}