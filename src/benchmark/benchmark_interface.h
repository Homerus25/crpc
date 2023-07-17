#pragma once

#include <cista/containers/string.h>
#include <cista/containers/vector.h>
#include <numeric>
#include "crpc/rpc_fn.h"

namespace data = cista::offset;

struct benchmark_interface {
  fn<data::string, data::string> say_hello;
  fn<double, data::vector<double>> average;
  fn<data::vector<double>, int> get_rand_nums;
  fn<data::vector<unsigned char>, data::vector<unsigned char>> send_rcv_large_data;
};

template<typename Server>
void register_benchmark_interface(Server &server) {
  server.reg(&benchmark_interface::say_hello, [](data::string ms){
    auto str = std::string("hello " + ms.str());
    return data::string(str);
  });
  server.reg(&benchmark_interface::average,
             [](const data::vector<double>& vec) {
               double avg = 0.;
               for(double element : vec)
                 avg += element / vec.size();
               return avg;
  });
  server.reg(&benchmark_interface::get_rand_nums,
             [](int size) {
               data::vector<double> ret(size);
               std::iota(ret.begin(), ret.end(), 0.);
               return ret;
  });
  server.reg(&benchmark_interface::send_rcv_large_data,
             [](data::vector<unsigned char> l_data) { return l_data; });
}