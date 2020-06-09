#include <iostream>

#include "crpc/client.h"

using crpc::client;
namespace data = cista::raw;

struct custom_client : public client {
  custom_client(std::string const& name, unsigned int const port)
      : client(name, port) {}

  fn<0, data::string> make_daytime_string_;
  fn<1, int, int, int> add_two_nums_;
  fn<2, void, data::string> print_;
  fn<3, void> print_no_args_;
};

int main(int argc, char* argv[]) {
  custom_client client("127.0.0.1", 2000);

  std::cout << client.make_daytime_string_();

  std::cout << client.add_two_nums_(5, 2) << "\n";
  std::cout << client.add_two_nums_(10, 100) << "\n";

  client.print_("hi");

  client.print_no_args_();

  return 0;
}
