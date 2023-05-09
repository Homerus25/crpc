#include "../benchmark_interface.h"
#include "crpc/http/rpc_http_client.h"

int main(int argc, char* argv[]) {
  rpc_http_client<benchmark_interface> client{std::string("http://127.0.0.1:9000/"), 9000u};
  auto resp = client.call(&benchmark_interface::say_hello, data::string("peter"));
}