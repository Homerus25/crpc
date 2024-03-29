#pragma once

#include "../rpc_async_client.h"
#include "../message.h"
#include "../log.h"
#include "../ticket_store.h"

#include "../../ws_client.h"

#include <memory>

template<typename Serializer>
struct ws_transport {
  explicit ws_transport(Receiver<Serializer> rec, std::string const& url = "127.0.0.1", unsigned int const port = 9000u)
      : receiver(rec), work_guard_(boost::asio::make_work_guard(ioc_))
  {
    client = std::make_unique<net::ws_client>(ioc_, url, std::to_string(port));
    client->run([&](boost::system::error_code ec) {
      if (ec) {
        LogErr("connect failed: ", ec.message());
        return;
      }
      Log("connected");
      isConnected = true;
      cv.notify_all();
    });
    client->on_fail([&](boost::system::error_code ec) {
      LogErr("client fail: ", ec.to_string());
    });

    client->on_msg([&](std::string const& msg, bool /* binary */) {
      receiver.processAnswer(Serializer::fromString(msg));
    });

    runner = std::thread([&](){ ioc_.run(); });

    std::unique_lock lk(mutex);
    cv.wait(lk, [&] { return isConnected; });
  }

  void send(Serializer::SerializedClientMessageContainer ms_buf) {
    auto bytes = reinterpret_cast<char const*>(ms_buf.data());
    auto bytes_end = bytes + ms_buf.size();
    std::string ms_string(bytes, bytes_end);

    client->send(ms_string, true);
  }

  void stop() {
    client->stop();
    work_guard_.reset();
    runner.join();
  }

private:
  Receiver<Serializer> receiver;
  boost::asio::io_context ioc_;
  std::thread runner;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
  std::unique_ptr<net::ws_client> client;
  std::condition_variable cv;
  std::mutex mutex;
  bool isConnected = false;
};

template<typename Interface, typename Serializer>
using rpc_ws_client = rpc_async_client<ws_transport<Serializer>, Interface, Serializer>;
