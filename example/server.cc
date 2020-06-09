#include "crpc/server.h"

#include <functional>
#include <iostream>

namespace data = cista::raw;

data::string make_daytime_string() {
  std::time_t now = time(nullptr);
  return std::ctime(&now);
}

int add_two_nums(int a, int b) {
  std::cout << "add two nums: " << a << " " << b << "\n";
  return a + b;
}

void print(data::string const& str) { std::cout << str << "\n"; }

void print_no_args() { std::cout << "print with no args!\n"; }

int main(int argc, char* argv[]) {
  auto server = crpc::server{2000};

  // std::function<data::string(void)> func = make_daytime_string;
  // server.registerFunction<data::string>(func);
  // server.registerFunction<data::string>(make_daytime_string);
  // server.reg<data::string(void)>(make_daytime_string);

  std::function<data::string(void)> func = make_daytime_string;
  // server.reg<std::function<data::string(void)>>(func);
  server.reg(func);
  // server.reg<data::string, void>(make_daytime_string);
  // server.reg(&(std::function<data::string(void)>{make_daytime_string}));

  std::function<int(int, int)> func2 = add_two_nums;
  // server.register_function_args<int, int, int>(func2);
  server.reg(func2);
  // server.register_function_args<int, int, int>(add_two_nums);

  std::function<void(data::string)> func3 = print;
  server.reg(func3);
  // server.register_function_no_return<data::string>(func3);

  std::function<void(void)> func4 = print_no_args;
  // server.register_function_no_return_no_parameter(func4);
  server.reg(func4);
  // server.reg(print_no_args);

  server.run();
  // server.test();

  return 0;
}
