#include "ws_client.h"

#include <iostream>
#include <queue>

#include "boost/asio.hpp"
#include "boost/asio/post.hpp"
#include "boost/beast/core.hpp"
#include "boost/beast/websocket.hpp"

#define return_on_error(what) \
  if (ec) {                   \
    if (cb) {                 \
      cb(ec);                 \
    }                         \
    return;                   \
  }

namespace asio = boost::asio;
namespace websocket = boost::beast::websocket;
using tcp = asio::ip::tcp;
using boost::system::error_code;

namespace net {

struct ws_client::impl : public boost::asio::coroutine,
                         public std::enable_shared_from_this<impl> {
  impl(asio::io_service& ios, std::string host,
       std::string port)
      : resolve_{ios},
        ws_{ios},
        host_{std::move(host)},
        port_{std::move(port)} {}

  void run(std::function<void(error_code)> const& cb) {
    stopped = false;
    loop(shared_from_this(), error_code{}, cb);
  }

#include "boost/asio/yield.hpp"
  void loop(std::shared_ptr<impl> const& me, error_code const ec,
            std::function<void(error_code)> const& cb = nullptr,
            tcp::resolver::results_type const& results = {}) {
    reenter(*this) {
      // Resolve endpoint.
      yield resolve_.async_resolve(
          host_, port_,
          [me, cb](error_code ec,
                   tcp::resolver::results_type const& endpoints) {
            me->loop(me, ec, cb, endpoints);
          });
      return_on_error("resolve");

      yield asio::async_connect(
          ws_.next_layer(), results.begin(), results.end(),
          std::bind(&impl::on_connect, this, me, std::placeholders::_1, cb));
      return_on_error("connect");

      // Websocket handshake.
      yield ws_.async_handshake(
          host_, "/", [me, cb](error_code ec) { me->loop(me, ec, cb); });
      return_on_error("ws handshake");

      // Callback on successful connection.
      if (cb) {
        cb(ec);
      }

      // Read incoming messages.
      while (true) {
        yield ws_.async_read(
            buffer_, [me](error_code ec, std::size_t) { me->loop(me, ec); });

        if (ec) {
          if (!stopped && on_fail_fn_) {
            on_fail_fn_(ec);
          }
          return;
        }

        if (on_msg_fn_) {
          on_msg_fn_(boost::beast::buffers_to_string(buffer_.data()),
                     ws_.got_binary());
        }
        buffer_.consume(buffer_.size());
      }
    }
  }
#include "boost/asio/unyield.hpp"

  void on_connect(std::shared_ptr<impl> const& me, error_code ec,
                  std::function<void(error_code)> const& cb = nullptr) {
    loop(me, ec, cb);
  }

  void send(std::string const& msg, bool binary) {
    asio::post(boost::beast::get_lowest_layer(ws_).get_executor(), [this, msg, binary]() {
      queue_.emplace(msg, binary);
      if (!send_active_) {
        send_next(shared_from_this());
      }
    });
  }

  void send_next(std::shared_ptr<impl> const& me) {
    if (queue_.empty()) {
      send_active_ = false;
      return;
    }

    auto const [msg, binary] = queue_.front();
    queue_.pop();

    auto copy = std::make_shared<std::string>(msg);
    ws_.binary(binary);
    ws_.async_write(
        boost::asio::buffer(*copy),
        [me, copy](error_code ec, std::size_t s) { me->send_next(me); });
    send_active_ = true;
  }

  void on_msg(std::function<void(std::string, bool /* binary */)> fn) {
    on_msg_fn_ = std::move(fn);
  }

  void on_fail(std::function<void(error_code)> fn) {
    on_fail_fn_ = std::move(fn);
  }

  void stop() {
    ws_.async_close(websocket::close_code::normal,
      [me = shared_from_this()](error_code ec) {
        me->stopped = true;
        if(ec) {
          std::cout << "error on close: " << ec.what() << std::endl;
        }
      });
  }

  std::string host_, port_;
  std::function<void(std::string, bool /* binary */)> on_msg_fn_;
  std::function<void(error_code)> on_fail_fn_;
  tcp::resolver resolve_;
  websocket::stream<tcp::socket> ws_;
  boost::beast::multi_buffer buffer_;
  std::queue<std::pair<std::string, bool>> queue_;
  bool send_active_{false};
  bool stopped{true};
};

ws_client::ws_client(asio::io_service& ios,
                     std::string const& host, std::string const& port)
    : impl_{std::make_shared<impl>(ios, host, port)} {}

ws_client::~ws_client() = default;

void ws_client::send(std::string const& msg, bool binary) {
  if (impl_) {
    impl_->send(msg, binary);
  }
}

void ws_client::run(const std::function<void(error_code)>& cb) {
  if (impl_) {
    impl_->run(cb);
  }
}

void ws_client::on_msg(std::function<void(std::string, bool)> fn) {
  if (impl_) {
    impl_->on_msg(std::move(fn));
  }
}

void ws_client::on_fail(std::function<void(error_code)> fn) {
  if (impl_) {
    impl_->on_fail(std::move(fn));
  }
}

void ws_client::stop() {
  if (impl_) {
    impl_->stop();
    impl_ = nullptr;
  }
}

}  // namespace net
