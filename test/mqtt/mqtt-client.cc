#define CRPC_LOG

#include "../../example/benchmark_interface.h"
#include "crpc/mqtt/rpc_mqtt_transport.h"

int main(int argc, char* argv[]) {
  boost::asio::io_context ioc;
  rpc_mqtt_client<benchmark_interface> client{ioc, std::string("127.0.0.1"), static_cast<std::uint16_t>(2000)};

  ioc.poll();
  std::thread t1([&](){
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> x
        = boost::asio::make_work_guard(ioc);
    ioc.run();
  });

  std::this_thread::sleep_for(std::chrono::seconds(2));

  std::string test_str("Hello Big Data Echo");

  auto res_hello = client.call(&benchmark_interface::say_hello, data::string("peter"));
  auto res_avg = client.call(&benchmark_interface::average, data::vector<double>({10., 20.}));
  auto res_echo = client.call(&benchmark_interface::send_rcv_large_data, data::vector<unsigned char>(test_str.begin(), test_str.end()));

  auto unpacked_hello = res_hello();
  auto unpacked_avg = res_avg();

  std::string const hl(unpacked_hello.begin(), unpacked_hello.end());

  if (hl != "hello peter" || unpacked_avg != 15)
    return 1;

  auto echo = res_echo();
  std::string const echoback(echo.begin(), echo.end());
  if (test_str != echoback)
    return 1;

  ioc.stop();
  t1.join();
  return 0;
}