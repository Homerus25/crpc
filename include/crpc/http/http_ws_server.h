#pragma once

#include "../rpc_server.h"
#include "../log.h"

#include "net/stop_handler.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

using namespace net;
using net::web_server;

template <typename Interface>
class http_ws_server : public rpc_server<Interface>  {
public:
  void run() {
    boost::asio::io_context ioc_;
    web_server webs_{ioc_};

    webs_.on_ws_msg([this](ws_session_ptr const& s, std::string const& msg,
                        ws_msg_type type) {
      Log("received: \"", msg, "\"");
      if (auto session = s.lock()) {
        auto response = this->process_message(msg);
        if (response.has_value()) {
          std::string resMs(response.value().begin(), response.value().end());
          session->send(resMs, type,
                        [](boost::system::error_code ec, std::size_t) {
                          if (ec) {
                            Log("send ec: ", ec.message());
                          }
                        });
        }
      }
    });

    webs_.on_ws_open(
        [](ws_session_ptr const& s, std::string const& target, bool ssl) {
          if (auto session = s.lock()) {
            Log("session open: " , session.get(), " ssl=", ssl);
          }
        });

    webs_.on_ws_close([](void* s) { Log("session close: ", s); });

    webs_.on_http_request([this](web_server::http_req_t const& req,
                                 web_server::http_res_cb_t const& cb, bool ssl) {
      auto x = req.body();
      Log("got ms: ", req.body());
      auto response = this->process_message(req.body());
      if (response.has_value()) {
        std::string resMs(response.value().begin(), response.value().end());
        return cb(string_response(req, resMs));
      }
      return cb(empty_response(req, boost::beast::http::status::ok, std::string_view()));
    });

    boost::system::error_code ec;
    webs_.init("127.0.0.1", "9000", ec);
    if (ec) {
      LogErr("init error: " , ec.message());
      return;
    }

    stop_handler const stop(ioc_, [&]() {
      webs_.stop();
      ioc_.stop();
    });

    Log("web server running on http://127.0.0.1:9000/");

    webs_.run();
    ioc_.run();
  }
};