#include "crpc/server.h"

#include <iostream>

namespace crpc {

struct c_int {
  int a_;
};

server::server(unsigned int port)
    : acceptor_(io_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                                                    port)),
      buffer_(1024) {}

void server::run() {
  while (true) {
    boost::asio::ip::tcp::socket socket(io_);
    acceptor_.accept(socket);
    listen_to_client(socket);
  }
}

void server::test() {
  struct test_str {
    int a_, b_;
  };
  test_str st{5, 2};
  params_ = cista::serialize(st);

  auto message = funcs_[1]();
  std::cout << *cista::deserialize<int>(message);
}

void server::listen_to_client(boost::asio::ip::tcp::socket& socket) {
  boost::system::error_code error;
  for (;;) {
    socket.read_some(boost::asio::buffer(buffer_), error);
    if (error) {
      std::cout << "error while reading!\n";
      // throw boost::system::system_error(error);
      return;
    }

    std::cout << "got a message: " << std::string_view(buffer_.data()) << "\n";

    params_.clear();
    params_.resize(
        std::distance((buffer_.begin() + sizeof(c_int)), buffer_.end()));
    std::copy(buffer_.begin() + sizeof(c_int), buffer_.end(), params_.begin());

    auto const func_num = cista::deserialize<c_int>(buffer_)->a_;
    if (func_num < funcs_.size()) {
      std::cout << "passed with function number: " << func_num << "!\n";
      std::cout << "size: " << funcs_.size() << "\n";

      auto const message = funcs_[func_num]();
      if (!message.empty())
        boost::asio::write(socket, boost::asio::buffer(message), error);
      else {
        std::vector<unsigned char> tmp;
        boost::asio::write(socket, boost::asio::buffer(tmp), error);
      }
    }

    if (error) {
      throw boost::system::system_error(error);
    }
  }
}

}  // namespace crpc