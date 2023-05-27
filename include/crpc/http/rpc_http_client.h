#pragma once

#include "../rpc_async_client.h"
#include "../message.h"
#include "../log.h"
#include "../ticket_store.h"
#include "boost/asio/post.hpp"

#include <cista/containers.h>
#include "net/http/client/http_client.h"
#include "net/tcp.h"

#include <memory>
#include <utility>
#include <functional>

struct http_transport {
  explicit http_transport(Receiver rec, std::string url, unsigned int const port)
    : receiver(rec), url_(std::move(url)), work_guard_(boost::asio::make_work_guard(ios_))
  {
    runner_ = std::thread([&](){ ios_.run(); });
  }

  void send(std::vector<unsigned char> ms_buf) {
    auto const ms_string = std::string(begin(ms_buf), end(ms_buf));
    auto s_req = std::make_shared<net::http::client::request>(net::http::client::request{
        url_, net::http::client::request::method::GET,
        net::http::client::request::str_map(), ms_string});

    ios_.post([this, s_req]() {
      auto client2 = net::http::client::make_http(ios_, url_);
      client2->query(*s_req, getLambda(s_req));
        });
  }

  net::http::client::basic_http_client<net::tcp>::callback getLambda(std::shared_ptr<net::http::client::request> req) {
    return [&, this](std::shared_ptr<net::tcp> client,
                  const net::http::client::response& res,
                  boost::system::error_code ec) {
      if (ec) {
        client->cancel();
        std::cout << "error: " << ec.message() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        //net::http::client::make_http(ios_, url_)->query(*req, getLambda(req));
        ios_.post([this, req]() {
          auto client2 = net::http::client::make_http(ios_, url_);
          //client2->query(req, std::bind(&http_transport::processRespond, this));
          client2->query(*req, getLambda(req));
        });

      } else {
        std::vector<unsigned char> dd(res.body.begin(), res.body.end());
        this->receiver.processAnswer(dd);
      }
    };
  }
/*
  void processRespond(std::shared_ptr<net::tcp>,
                           const http::client::response& res,
                           boost::system::error_code ec) {
    //return [this](std::shared_ptr<tcp>,
    //                                http::client::response const& res,
    //                                boost::system::error_code ec) {
            if (ec) {
              std::cout << "error: " << ec.message() << "\n";
            } else {
              std::vector<unsigned char> dd(res.body.begin(), res.body.end());
              this->receiver.processAnswer(dd);
            }
    //      };
  }
*/
  void stop() {
        //client->cancel();
       // client.reset();
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
  //std::shared_ptr<net::http::client::http> client;
};

template<typename Interface>
using rpc_http_client = rpc_async_client<http_transport, Interface>;
