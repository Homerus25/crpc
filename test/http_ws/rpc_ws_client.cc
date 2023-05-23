#define CRPC_LOG

#include "crpc/http/rpc_ws_client.h"
#include "../../src/benchmark/benchmark_interface.h"

int main(int argc, char* argv[]) {
  boost::asio::io_context ioc;
  rpc_ws_client<benchmark_interface> client{ioc, std::string("127.0.0.1"), 9000u};

  ioc.poll();
  std::thread t1([&](){
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> x
        = boost::asio::make_work_guard(ioc);
    ioc.run();
  });

  std::string testStr("Hello Big Data Echo");

  auto resHello = client.call(&benchmark_interface::say_hello, data::string("peter"));
  auto resAvg = client.call(&benchmark_interface::average, data::vector<double>({10., 20.}));
  auto resEcho = client.call(&benchmark_interface::send_rcv_large_data, data::vector<unsigned char>(testStr.begin(), testStr.end()));

  auto unpackedHello = resHello();
  auto unpackedAvg = resAvg();

  std::string hl(unpackedHello.begin(), unpackedHello.end());

  if (hl != "hello peter" || unpackedAvg != 15)
    return 1;

  auto echo = resEcho();
  std::string echoback(echo.begin(), echo.end());
  if (testStr != echoback)
    return 1;


  ioc.stop();
  t1.join();
  return 0;
}