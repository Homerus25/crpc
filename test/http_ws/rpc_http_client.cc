#define CRPC_LOG

#include "crpc/http/rpc_http_client.h"
#include "../../src/benchmark/benchmark_interface.h"
#include "crpc/serialization/cista.h"

int main(int argc, char* argv[]) {
  rpc_http_client<benchmark_interface, CistaSerialzer> client{std::string("http://127.0.0.1:9000/"), 9000u};

  std::string testStr("Hello Big Data Echo");

  auto resHello = client.call(&benchmark_interface::say_hello, data::string("peter"));
  auto resAvg = client.call(&benchmark_interface::average, data::vector<double>({10., 20.}));
  auto resEcho = client.call(&benchmark_interface::send_rcv_large_data, data::vector<unsigned char>(testStr.begin(), testStr.end()));

  auto unpackedHello = resHello();
  std::cout << unpackedHello << std::endl;
  auto unpackedAvg = resAvg();
  std::cout << unpackedAvg << std::endl;

  std::string hl(unpackedHello.begin(), unpackedHello.end());

  if (hl != "hello peter" || unpackedAvg != 15)
    return 1;

  auto echo = resEcho();
  std::string echoback(echo.begin(), echo.end());
  if (testStr != echoback)
    return 1;

  client.stop();
  return 0;
}