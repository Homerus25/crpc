#pragma once

#include <nghttp2/asio_http2_client.h>
#include "crpc/log.h"
#include "crpc/rpc_async_client.h"

#include <memory>
#include <algorithm>

template<typename Serializer>
struct http2_transport {
  explicit http2_transport(Receiver<Serializer> rec, std::string url = "localhost", unsigned int const port = 3000u)
    : receiver(rec), sess(io_service, "localhost", std::to_string(port))
  {
    boost::system::error_code ec;

    runner_ = std::thread([&](){ io_service.run(); });

    // connect to localhost:3000
    //nghttp2::asio_http2::client::session sess(io_service, "localhost", std::to_string(port));

    sess.on_connect([&](boost::asio::ip::tcp::resolver::iterator endpoint_it) {
      isConnected = true;
    });

    while (!isConnected) {
      Log("wait for connection");
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    sess.on_error([](const boost::system::error_code& ec) {
      std::cerr << "error: " << ec.message() << std::endl;
    });

    //io_service.run();
  }

  void send(Serializer::SerializedClientMessageContainer ms_buf) {
    //auto ms_buf_str = Serializer::toString(ms_buf);
    boost::asio::post(io_service, [&, ms_buf_str = Serializer::toString(ms_buf)]{
      boost::system::error_code ec;
      auto req = sess.submit(ec, "GET", "http://localhost:3000/", ms_buf_str);

      req->on_response([&](const nghttp2::asio_http2::client::response& res) {
        // this->receiver.processAnswer(Serializer::fromString(res.body));

        /*
        // print status code and response header fields.
        std::cout << "HTTP/2 " << res.status_code() << std::endl;
        for (auto& kv : res.header()) {
          std::cout << kv.first << ": " << kv.second.value << "\n";
        }
        std::cout << std::endl;
        */
        std::shared_ptr<std::vector<std::byte>> bucket =
            std::make_shared<std::vector<std::byte>>();
        // incoming_data.push_back(bucket);

        // res.on_data([bucket = std::vector<std::byte>()/*std::unique_ptr<std::vector<std::byte>>() /*std::shared_ptr<std::vector<std::byte>>()/*std::make_shared<std::vector<std::byte>>()*/, this](const uint8_t* data, std::size_t len) {
        res.on_data([bucket, this](const uint8_t* data, std::size_t len) {
          // std::cerr.write(reinterpret_cast<const char*>(data), len);
          // std::cerr << std::endl;

          if (len != 0) {
            const auto start = reinterpret_cast<std::byte const*>(data);
            auto end = start + len;
            // auto dataVec = std::vector<std::byte>(start, end);
            bucket->reserve(bucket->size() + len);
            std::copy(start, end,
                      std::back_inserter(*bucket));  // bucket->end());

            /*
            for(int i=0; i<len; ++i) {
              bucket.push_back();
            }
             */
          } else {
            this->receiver.processAnswer(*bucket);
            // delete bucket!
            // bucket.release();
          }
        });
      });
      Log("request sended");
    });
  }

  void stop() {
    sess.shutdown();
    runner_.join();
  }

private:
  boost::asio::io_service io_service;
  nghttp2::asio_http2::client::session sess;
  std::atomic<bool> isConnected = false;
  std::thread runner_;
  //std::vector<std::shared_ptr<std::vector<std::byte>>> incoming_data;
  Receiver<Serializer> receiver;
};

template<typename Interface, typename Serializer>
using rpc_http2_client = rpc_async_client<http2_transport<Serializer>, Interface, Serializer>;