#pragma once

#include "../rpc_server.h"
#include "../log.h"

#include "net/stop_handler.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

#include <memory>
#include <thread>

using namespace net;
using net::web_server;

template <typename Interface, typename Serializer>
class http_ws_server : public rpc_server<Interface, Serializer>  {
public:
  http_ws_server() {
    webs_ = std::unique_ptr<web_server>(new web_server{ioc_});
    webs_->set_request_body_limit(1024 * 64 + 1024);

    webs_->on_ws_msg([this](ws_session_ptr const& s, std::string const& msg,
                        ws_msg_type type) {
      Log("received: \"", msg, "\"");
      if (auto session = s.lock()) {
        auto response = this->process_message(msg);
        if (response.has_value()) {
          session->send(response.value(), type,
                        [](boost::system::error_code ec, std::size_t) {
                          if (ec) {
                            LogErr("send ec: ", ec.message());
                          }
                        });
        }
      }
    });

    webs_->on_ws_open(
        [](ws_session_ptr const& s, std::string const& target, bool ssl) {
          if (auto session = s.lock()) {
            Log("session open: " , session.get(), " ssl=", ssl);
          }
        });

    webs_->on_ws_close([](void* s) { Log("session close: ", s); });

    webs_->on_http_request([this](http_req_t const& req,
                                 http_res_cb_t const& cb, bool ssl) {
      boost::ignore_unused(ssl);
      auto response = this->process_message(req.body());
      if (response.has_value()) {
        return cb(string_response(req, Serializer::toString(response.value())/*resMs*/, boost::beast::http::status::ok, "binary"));
      }
      LogErr("server wants to send empty response!");
      return cb(empty_response(req, boost::beast::http::status::ok));
    });

    boost::system::error_code ec;
    webs_->init("127.0.0.1", "9000", ec);
    if (ec) {
      LogErr("init error: " , ec.message());
      return;
    }
    Log("web server running on http://127.0.0.1:9000/");


    webs_->run();
  }

  void run(int threads_count) {
    for(int i=0; i<threads_count; ++i)
      runner.emplace_back([&](){ ioc_.run(); });
  }

  void stop() {
    Log("server canceled connection");
    webs_->stop();
    for(auto& t: runner)
      t.join();
  }

  void block() {
    ioc_.run();
  }

private:
  boost::asio::io_context ioc_;
  std::unique_ptr<web_server> webs_;
  std::vector<std::thread> runner;
};