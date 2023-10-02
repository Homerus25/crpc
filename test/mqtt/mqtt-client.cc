#define CRPC_LOG

#include "../../src/benchmark/benchmark_interface.h"
#include "crpc/mqtt/rpc_mqtt_transport.h"
#include "crpc/serialization/zpp_bits_serialization.h"

int main(int argc, char* argv[]) {
  rpc_mqtt_client<benchmark_interface<ZppBitsSerializer>, ZppBitsSerializer> client{std::string("127.0.0.1"), static_cast<std::uint16_t>(2000)};
  std::string test_str("Hello Big Data Echo");

  auto res_hello = client.call(&benchmark_interface<ZppBitsSerializer>::say_hello, std::string("peter"));
  auto res_avg = client.call(&benchmark_interface<ZppBitsSerializer>::average, std::vector<double>({10., 20.}));
  auto res_echo = client.call(&benchmark_interface<ZppBitsSerializer>::send_rcv_large_data, std::vector<unsigned char>(test_str.begin(), test_str.end()));

  auto unpacked_hello = res_hello();
  auto unpacked_avg = res_avg();

  std::string const hl(unpacked_hello.begin(), unpacked_hello.end());

  if (hl != "hello peter" || unpacked_avg != 15)
    return 1;

  auto echo = res_echo();
  std::string const echoback(echo.begin(), echo.end());
  if (test_str != echoback)
    return 1;

  client.stop();
  return 0;
}