#pragma once

#include <functional>
#include <memory>
#include <string>

#include "boost/system/error_code.hpp"

#include "boost/asio/coroutine.hpp"
#include "boost/asio/io_service.hpp"

namespace net {

    struct ws_client {
        ws_client(boost::asio::io_service& ios, std::string const& host, std::string const& port);
        ~ws_client();

        void run(const std::function<void(boost::system::error_code)>&);
        void send(std::string const&, bool binary);
        void on_msg(std::function<void(std::string, bool /* binary */)>);
        void on_fail(std::function<void(boost::system::error_code)>);
        void stop();

        struct impl;
        std::shared_ptr<impl> impl_;
    };

}  // namespace net
