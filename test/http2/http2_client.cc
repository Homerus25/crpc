#define CRPC_LOG

#include "crpc/http2/http2_client.h"
#include "../../src/benchmark/benchmark_interface.h"
#include "crpc/serialization/zpp_bits_serialization.h"

int main(int argc, char* argv[]) {
  rpc_http2_client<benchmark_interface<ZppBitsSerializer>, ZppBitsSerializer> client{};//{std::string("http://127.0.0.1:9000/"), 9000u};

  std::string testStr("Hello Big Data Echo");

  auto resHello = client.call(&benchmark_interface<ZppBitsSerializer>::say_hello, std::string("peter"));
  auto resAvg = client.call(&benchmark_interface<ZppBitsSerializer>::average, std::vector<double>({10., 20.}));
  auto resEcho = client.call(&benchmark_interface<ZppBitsSerializer>::send_rcv_large_data, std::vector<unsigned char>(testStr.begin(), testStr.end()));

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