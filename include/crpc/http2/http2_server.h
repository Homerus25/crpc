#pragma once

#include "../rpc_server.h"
#include <nghttp2/asio_http2_server.h>

template <typename Interface, typename Serializer>
class http2_server : public rpc_server<Interface, Serializer> {
public:
  http2_server() {
    server.handle("/",
      [&](const nghttp2::asio_http2::server::request& req,
         const nghttp2::asio_http2::server::response& res) {
                    std::shared_ptr<std::vector<std::byte>> bucket = std::make_shared<std::vector<std::byte>>();

        req.on_data([&res, bucket, this](const uint8_t *data, std::size_t size) {
          Log("got data");
          if(size != 0) {
            const auto start =
                reinterpret_cast<std::byte const*>(data);
            auto end = start + size;
            // auto dataVec = std::vector<std::byte>(start, end);
            bucket->reserve(bucket->size() + size);
            std::copy(start, end, std::back_inserter(*bucket));// bucket->end());
          }
          else {
            Log("calc respond");
            auto response = this->process_message(*bucket);
            if (response.has_value()) {
              res.write_head(200);
              res.end(Serializer::toString(response.value()));
            }
            res.write_head(500);
            res.end();
            Log("finished respond");
          }
        });

        //res.write_head(200);
        //res.end("hello, world\n");
      });
  }

  void run(int threads_count) {
    //for(int i=0; i<threads_count; ++i)
    //  runner.emplace_back([&](){ ioc_.run(); });
    server.num_threads(threads_count);

    boost::system::error_code ec;
    if (server.listen_and_serve(ec, "localhost", "3000", true)) {
      std::cerr << "error: " << ec.message() << std::endl;
    }
  }

  void stop() {
    Log("server canceled connection");
    server.stop();
    server.join();
  }

  void run_blocked(int threads_count) {
    //ioc_.run();
    //server.io_services()[0]->run();
    server.num_threads(threads_count);

    boost::system::error_code ec;
    if (server.listen_and_serve(ec, "localhost", "3000", false)) {
      std::cerr << "error: " << ec.message() << std::endl;
    }
  }

private:
  //boost::asio::io_context ioc_;
  nghttp2::asio_http2::server::http2 server;
  //std::vector<std::thread> runner;
};