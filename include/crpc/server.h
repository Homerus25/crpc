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

  template <typename returnType>
  void register_function_no_args(std::function<returnType(void)> funcP) {
    funcs_.emplace_back([=]() { return stub_no_arguments<returnType>(funcP); });
  }

  template <typename returnType, class... ArgTypes>
  void register_function_args(std::function<returnType(ArgTypes...)>& funcP) {
    funcs_.emplace_back(
        [&, funcP]() { return stub<returnType, ArgTypes...>(funcP, params_); });
  }

  template <class... ArgTypes>
  void register_function_no_return(std::function<void(ArgTypes...)>& funcP) {
    funcs_.emplace_back([&, funcP]() {
      stub_no_return(funcP, params_);
      return std::vector<unsigned char>();
    });
  }

  void register_function_no_return_no_parameter(
      std::function<void(void)>& funcP) {
    funcs_.emplace_back([&, funcP]() {
      funcP();
      return std::vector<unsigned char>();
    });
  }

  template <typename retT, typename... ArgsT>
  void reg(std::function<retT(ArgsT...)>& func) {
    if constexpr (std::is_void_v<retT> && sizeof...(ArgsT) == 0) {
      register_function_no_return_no_parameter(func);
    } else if constexpr (std::is_void_v<retT> && sizeof...(ArgsT) > 0) {
      register_function_no_return(func);
    } else if constexpr (!std::is_void_v<retT> && sizeof...(ArgsT) == 0) {
      register_function_no_args(func);
    } else if constexpr (!std::is_void_v<retT> && sizeof...(ArgsT) > 0) {
      register_function_args(func);
    } else {
      static_assert("No valid registration!");
    }
  }

private:
  void listen_to_client(boost::asio::ip::tcp::socket&);

  template <typename RetType>
  static std::vector<unsigned char> stub_no_arguments(
      std::function<RetType(void)> funcPtr) {
    auto ret = funcPtr();
    return cista::serialize(ret);
  }

  template <typename RetType, class... ArgumentTypes>
  static std::vector<unsigned char> stub(
      std::function<RetType(ArgumentTypes...)> funcPtr,
      std::vector<unsigned char> args_buf) {
    auto const params =
        *(cista::deserialize<std::tuple<ArgumentTypes...>>(args_buf));
    RetType ret = std::apply(funcPtr, params);
    return cista::serialize(ret);
  }

  template <class... ArgumentTypes>
  static void stub_no_return(std::function<void(ArgumentTypes...)> func_ptr,
                             std::vector<unsigned char> args_buf) {
    auto const params =
        *(cista::deserialize<std::tuple<ArgumentTypes...>>(args_buf));
    std::apply(func_ptr, params);
  }

  boost::asio::io_context io_;
  boost::asio::ip::tcp::acceptor acceptor_;

  std::vector<std::function<std::vector<unsigned char>(void)>> funcs_;

  std::vector<char> buffer_;
  std::vector<unsigned char> params_;
};

}  // namespace crpc
