#pragma once

#include <cista/containers/string.h>
#include <cista/containers/vector.h>
#include <numeric>
#include "crpc/rpc_fn.h"

#include "crpc/serialization/cista.h"

namespace data = cista::offset;

template<typename Serializer>
struct benchmark_interface {
  fn<std::string, std::string> say_hello;
  fn<double, std::vector<double>> average;
  fn<std::vector<double>, int> get_rand_nums;
  fn<std::vector<unsigned char>, std::vector<unsigned char>> send_rcv_large_data;
};

template<>
struct benchmark_interface<CistaSerialzer> {
  fn<data::string, data::string> say_hello;
  fn<double, data::vector<double>> average;
  fn<data::vector<double>, int> get_rand_nums;
  fn<data::vector<unsigned char>, data::vector<unsigned char>> send_rcv_large_data;
};

template<typename Server, typename Serializer>
void register_benchmark_interface(Server &server) {
  if constexpr (std::is_same_v<Serializer, CistaSerialzer>) {
    server.reg(&benchmark_interface<CistaSerialzer>::say_hello, [](data::string ms) {
      auto str = std::string("hello " + ms.str());
      return data::string(str);
    });
    server.reg(&benchmark_interface<CistaSerialzer>::average,
               [](const data::vector<double>& vec) {
                 double avg = 0.;
                 for (double element : vec) avg += element / vec.size();
                 return avg;
               });
    server.reg(&benchmark_interface<CistaSerialzer>::get_rand_nums, [](int size) {
      data::vector<double> ret(size);
      std::iota(ret.begin(), ret.end(), 0.);
      return ret;
    });
    server.reg(&benchmark_interface<CistaSerialzer>::send_rcv_large_data,
               [](data::vector<unsigned char> l_data) { return l_data; });
  }
  else {
    server.reg(&benchmark_interface<Serializer>::say_hello, [](std::string ms) {
      return std::string("hello " + ms);
    });
    server.reg(&benchmark_interface<Serializer>::average,
               [](const std::vector<double>& vec) {
                 double avg = 0.;
                 for (double element : vec) avg += element / vec.size();
                 return avg;
               });
    server.reg(&benchmark_interface<Serializer>::get_rand_nums, [](int size) {
      data::vector<double> ret(size);
      std::iota(ret.begin(), ret.end(), 0.);
      return ret;
    });
    server.reg(&benchmark_interface<Serializer>::send_rcv_large_data,
               [](std::vector<unsigned char> l_data) { return l_data; });
  }
}