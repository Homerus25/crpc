#include "Client.h"

crpc::Client::Client(const std::string& name, unsigned int port)
    : endpoints(boost::asio::ip::address::from_string(name), port)
    , buffer(1024)
{}

