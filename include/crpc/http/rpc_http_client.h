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
#include <queue>

template<typename Serializer>
struct http_transport {
  explicit http_transport(Receiver<Serializer> rec, std::string url = "http://127.0.0.1:9000/", unsigned int const port = 9000u)
      : receiver(rec), url_(std::move(url)), work_guard_(boost::asio::make_work_guard(ios_))
  {
    runner_ = std::thread([&](){ ios_.run(); });
    http_client = net::http::client::make_http(ios_, url_);
    http_client->use_timeout_ = false;
    http_client->connect([&](net::tcp::tcp_ptr c, boost::system::error_code ec) {
      if(ec)
        LogErr("not connected: ", ec.what());
      else {
        Log("connected!");
        if (isConnected) {
          LogErr("connected double!");
        }
        isConnected = true;
      }
    });

    while (!isConnected) {
      Log("wait for connection");
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  void send(Serializer::SerializedContainer ms_buf) {
    std::string ms_string(begin(ms_buf), end(ms_buf));

    auto s_req = std::make_shared<net::http::client::request>(
        net::http::client::request{
            url_, net::http::client::request::method::GET,
            {},
            ms_string}
    );

    ios_.post([this, s_req]() {
      queue_.emplace(s_req);
      send_next();
    });
  }

  void send_next() {
    if (!queryInProgress && !queue_.empty()) {
      auto req = queue_.front();
      queue_.pop();
      queryInProgress = true;
      http_client->query(*req, getLambda());
    }
  }

  net::http::client::basic_http_client<net::tcp>::callback getLambda() {
    return [&, this](std::shared_ptr<net::tcp> client,
                     const net::http::client::response& res,
                     boost::system::error_code ec) {
      if (ec) {
        LogErr("error: ", ec.message());
      } else {
        typename Serializer::SerializedContainer dd(res.body.begin(), res.body.end());
        this->receiver.processAnswer(dd);
        queryInProgress = false;
        ios_.post([this](){
          send_next();
        });
      }
    };
  }

  void stop() {
    Log("client canceled connection");
    http_client.reset();
    work_guard_.reset();
    runner_.join();
    ios_.poll();
    isConnected = false;
  }

private:
  Receiver<Serializer> receiver;
  boost::asio::io_service ios_;
  std::string url_;
  std::thread runner_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
  std::shared_ptr<net::http::client::http> http_client;
  std::atomic<bool> isConnected = false;
  bool queryInProgress = false;
  std::queue<std::shared_ptr<net::http::client::request>> queue_;
};

template<typename Interface, typename Serializer>
using rpc_http_client = rpc_async_client<http_transport<Serializer>, Interface, Serializer>;
