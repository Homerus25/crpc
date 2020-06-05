#include "Client.h"

crpc::Client* crpc::Client::self;

crpc::Client::Client(const std::string& name, unsigned int port)
    : endpoints(boost::asio::ip::address::from_string(name), port)
    , buffer(1024)
    , socket(io)
{
    self = this;
    socket.connect(endpoints);
}

