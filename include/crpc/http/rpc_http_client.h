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
  explicit http_transport(std::string url, unsigned int const port)
    : url_(std::move(url))
  {
    http_client_ = net::http::client::make_http(ios_, url_);
    http_client_->connect([](net::tcp::tcp_ptr, boost::system::error_code ec){
      if(ec)
        LogErr("error occured: ", ec);
    });

    ios_.poll();

    std::thread t1([&](){
      boost::asio::executor_work_guard<boost::asio::io_context::executor_type> x
          = boost::asio::make_work_guard(ios_);
      ios_.run();
    });
    t1.detach();
  }

  std::future<std::vector<unsigned char>> send(unsigned fn_idx,
                                               std::vector<unsigned char> const& params) {
    message ms{
        ts_.nextNumber(), fn_idx,
        cista::offset::vector<unsigned char>(params.begin(), params.end())};
    auto future = ts_.emplace(ms.ticket_);

    auto const ms_buf = cista::serialize(ms);
    auto const ms_string = std::string(begin(ms_buf), end(ms_buf));
    net::http::client::request req{
          url_,
        net::http::client::request::method::GET,
          {},
          ms_string
      };

      //http_client_
    net::http::client::make_http(ios_, url_)
          ->query(req, [this](std::shared_ptr<net::tcp>, net::http::client::response const& res, boost::system::error_code ec) {
            if (ec) {
              std::cout << "error: " << ec.message() << "\n";
            } else {
              cista::offset::vector<unsigned char> dd(res.body.begin(), res.body.end());
              auto ms = cista::deserialize<message>(dd);
              this->ts_.setValue(ms->ticket_, ms->payload_);
            }
          });

    return future;
  }

  void stop() {
    ios_.stop();
  }

private:
  boost::asio::io_service ios_;
  ticket_store ts_;
  const std::string url_;
  std::shared_ptr<net::http::client::http> http_client_;
};

template<typename Interface>
using rpc_http_client = rpc_async_client<http_transport, Interface>;
