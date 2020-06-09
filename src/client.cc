#include "crpc/client.h"

namespace crpc {

client* client::self_;

client::client(std::string const& name, unsigned int const port)
    : endpoints_(boost::asio::ip::address::from_string(name), port),
      buffer_(1024) {
  self_ = this;
  socket_.connect(endpoints_);
}

}  // namespace crpc