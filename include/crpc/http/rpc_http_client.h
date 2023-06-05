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
#include <condition_variable>

struct http_transport {
  explicit http_transport(Receiver rec, std::string url, unsigned int const port)
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
      Log("send too early");
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  void send(std::vector<unsigned char> ms_buf) {
    auto const ms_string = std::string(begin(ms_buf), end(ms_buf));
    auto s_req = std::make_shared<net::http::client::request>(
        net::http::client::request{
            url_, net::http::client::request::method::GET,
      {{"Connection", "Keep-Alive"}},
      ms_string}
    );

    {
      std::unique_lock lk(mutex);
      cv.wait(lk, [&] { return !queryInProgress; });
      queryInProgress = true;
    }
    ios_.post([=](){
      http_client->query(*s_req, getLambda(ms_string));
      });
  }

  net::http::client::basic_http_client<net::tcp>::callback getLambda(std::string ms_string) {
    return [&, this](std::shared_ptr<net::tcp> client,
                  const net::http::client::response& res,
                  boost::system::error_code ec) {
      if (ec) {
        LogErr("error: ", ec.message());
      } else {
        std::vector<u_int8_t> dd(res.body.begin(), res.body.end());
        this->receiver.processAnswer(dd);
        queryInProgress = false;
        cv.notify_one();
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
  Receiver receiver;
  boost::asio::io_service ios_;
  ticket_store ts_;
  std::string url_;
  std::thread runner_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
  std::shared_ptr<net::http::client::http> http_client;
  std::atomic<bool> isConnected = false;
  std::condition_variable cv;
  std::mutex mutex;
  bool queryInProgress = false;
};

template<typename Interface>
using rpc_http_client = rpc_async_client<http_transport, Interface>;
