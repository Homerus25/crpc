#pragma once

#include <functional>
#include <type_traits>
#include <vector>

#include "boost/asio.hpp"

#include "cista/serialization.h"

namespace crpc {

struct server {
  explicit server(unsigned int port);

  void run();
  void test();

  template <typename ReturnType>
  void register_function_no_args(std::function<ReturnType(void)> f) {
    funcs_.emplace_back([f]() { return stub_no_arguments<ReturnType>(f); });
  }

  template <typename ReturnType, typename... ArgTypes>
  void register_function_args(std::function<ReturnType(ArgTypes...)>& f) {
    funcs_.emplace_back(
        [f, this]() { return stub<ReturnType, ArgTypes...>(f, params_); });
  }

  template <typename... ArgTypes>
  void register_function_no_return(std::function<void(ArgTypes...)>& f) {
    funcs_.emplace_back([this, f]() {
      stub_no_return(f, params_);
      return std::vector<unsigned char>();
    });
  }

  void register_function_no_return_no_parameter(std::function<void(void)>& f) {
    funcs_.emplace_back([f]() {
      f();
      return std::vector<unsigned char>();
    });
  }

  template <typename RetT, typename... ArgsT>
  void reg(std::function<RetT(ArgsT...)>& func) {
    if constexpr (std::is_void_v<RetT> && sizeof...(ArgsT) == 0) {
      register_function_no_return_no_parameter(func);
    } else if constexpr (std::is_void_v<RetT> && sizeof...(ArgsT) > 0) {
      register_function_no_return(func);
    } else if constexpr (!std::is_void_v<RetT> && sizeof...(ArgsT) == 0) {
      register_function_no_args(func);
    } else if constexpr (!std::is_void_v<RetT> && sizeof...(ArgsT) > 0) {
      register_function_args(func);
    } else {
      static_assert("No valid registration!");
    }
  }

private:
  void listen_to_client(boost::asio::ip::tcp::socket&);

  template <typename RetType>
  static std::vector<unsigned char> stub_no_arguments(
      std::function<RetType(void)> f) {
    auto const ret = f();
    return cista::serialize(ret);
  }

  template <typename RetType, typename... ArgumentTypes>
  static std::vector<unsigned char> stub(
      std::function<RetType(ArgumentTypes...)> f,
      std::vector<unsigned char> args_buf) {
    auto const ret = std::apply(
        f, *(cista::deserialize<std::tuple<ArgumentTypes...>>(args_buf)));
    return cista::serialize(ret);
  }

  template <typename... ArgumentTypes>
  static void stub_no_return(std::function<void(ArgumentTypes...)> f,
                             std::vector<unsigned char> args_buf) {
    std::apply(f,
               *(cista::deserialize<std::tuple<ArgumentTypes...>>(args_buf)));
  }

  boost::asio::io_context io_;
  boost::asio::ip::tcp::acceptor acceptor_;

  std::vector<std::function<std::vector<unsigned char>(void)>> funcs_;

  std::vector<char> buffer_;
  std::vector<unsigned char> params_;
};

}  // namespace crpc
