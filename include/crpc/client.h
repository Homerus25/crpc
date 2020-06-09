#pragma once

#include <iostream>

#include "boost/asio.hpp"

#include "cista/serialization.h"

namespace crpc {

struct client {
  client(std::string const& name, unsigned int port);

  template <int procName, typename resultType>
  resultType call_no_parameter();

  template <int procName, typename resultType, class... ArgTypes>
  resultType call(ArgTypes...);

  template <int procName, class... ArgTypes>
  void call_no_return(ArgTypes...);

  template <int procName>
  void call_no_return_args();

  template <int ProcName, typename RetT, typename... ArgsT>
  struct fn {
    RetT operator()(ArgsT... args) {
      if constexpr (!std::is_void_v<RetT> && sizeof...(ArgsT) > 0) {
        return self_->call<ProcName, RetT, ArgsT...>(args...);
      } else if constexpr (!std::is_void_v<RetT> && sizeof...(ArgsT) == 0) {
        return self_->call_no_parameter<ProcName, RetT>();
      } else if constexpr (std::is_void_v<RetT> && sizeof...(ArgsT) > 0) {
        self_->call_no_return<ProcName, ArgsT...>(args...);
      } else if constexpr (std::is_void_v<RetT> && sizeof...(ArgsT) == 0) {
        self_->call_no_return_args<ProcName>();
      } else {
        static_assert("No valid call!");
      }
    }
  };

protected:
  static client* self_;

  boost::asio::io_context io_;
  boost::asio::ip::tcp::endpoint endpoints_;

  std::vector<char> buffer_;

  boost::asio::ip::tcp::socket socket_{io_};

  struct c_int {
    int a;
  };

  template <typename T>
  void append_vector(std::vector<T>& a, std::vector<T>& b) {
    auto offset = a.size();
    a.resize(a.size() + b.size());
    std::copy(b.begin(), b.end(), a.begin() + offset);
  }

  template <typename T>
  std::vector<unsigned char> serialize(T arg) {
    std::vector<unsigned char> buf = cista::serialize(arg);
    return buf;
  }

  template <typename Arg1, typename... Args>
  std::vector<unsigned char> serialize(Arg1 arg1, Args... args) {
    std::vector<unsigned char> buf = serialize(arg1);
    auto buf2 = serialize(args...);
    append_vector(buf, buf2);  // maybe there is a more performant way!?
    return buf;
  }
};

template <int ProcName, typename ResultType>
ResultType client::call_no_parameter() {
  std::vector<unsigned char> bu = serialize(ProcName);
  write(socket_, boost::asio::buffer(bu));

  socket_.read_some(boost::asio::buffer(buffer_));
  return *cista::offset::deserialize<ResultType>(buffer_);
}

template <int ProcName, typename ResultType, class... ArgTypes>
ResultType client::call(ArgTypes... args) {
  std::vector<unsigned char> bu = serialize(ProcName, args...);
  write(socket_, boost::asio::buffer(bu));

  socket_.read_some(boost::asio::buffer(buffer_));
  return *cista::offset::deserialize<ResultType>(buffer_);
}

template <int ProcName, class... ArgTypes>
void client::call_no_return(ArgTypes... args) {
  auto const bu = serialize(ProcName, std::forward<ArgTypes>(args)...);
  write(socket_, boost::asio::buffer(bu));
}

template <int ProcName>
void client::call_no_return_args() {
  std::vector<unsigned char> bu = serialize(ProcName);
  write(socket_, boost::asio::buffer(bu));
}

}  // namespace crpc
