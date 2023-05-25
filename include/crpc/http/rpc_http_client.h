#pragma once

#include "../rpc_async_client.h"
#include "../message.h"
#include "../log.h"
#include "../ticket_store.h"
#include "boost/asio/post.hpp"

#include <memory>
#include <utility>

#include <cista/containers.h>
#include "net/http/client/http_client.h"
#include "net/tcp.h"

struct http_transport {
  explicit http_transport(Receiver rec, std::string url, unsigned int const port)
    : receiver(rec), url_(std::move(url)), work_guard_(boost::asio::make_work_guard(ios_))
  {
    runner_ = std::thread([&](){ ios_.run(); });
  }

  void send(std::vector<unsigned char> ms_buf) {
    auto const ms_string = std::string(begin(ms_buf), end(ms_buf));
    net::http::client::request req{
        url_,
        net::http::client::request::method::GET,
        net::http::client::request::str_map(),
        ms_string
    };

    net::http::client::make_http(ios_, url_)
        ->query(req, [this](std::shared_ptr<net::tcp>, net::http::client::response const& res, boost::system::error_code ec) {
          if (ec) {
            std::cout << "error: " << ec.message() << "\n";
          } else {
            std::vector<unsigned char> dd(res.body.begin(), res.body.end());
            receiver.processAnswer(dd);
          }
        });
  }

  void stop() {
    work_guard_.reset();
    ios_.stop();
    runner_.join();
  }

private:
  Receiver receiver;
  boost::asio::io_service ios_;
  ticket_store ts_;
  std::string url_;
  std::thread runner_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
};

template<typename Interface>
using rpc_http_client = rpc_async_client<http_transport, Interface>;
