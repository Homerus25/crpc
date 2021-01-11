#pragma once

#include "rpc_async_client.h"

#include "message.h"
#include "deserialize_helper.h"
#include "boost/beast/core.hpp"
#include <boost/beast/websocket.hpp>
#include "boost/asio.hpp"

#include <cstdlib>
#include <thread>
#include <functional>
#include <boost/asio/spawn.hpp>

namespace beast = boost::beast;
//namespace net = boost::asio;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;

struct async_thread_websocket_transport {
    explicit async_thread_websocket_transport(std::string const& name, unsigned int const port)
            : endpoints_(boost::asio::ip::address::from_string(name), port)
            , ticket_num_(0)
            , is_reading(false)
            , ws(boost::asio::make_strand(io_))
    {
        ws.next_layer().connect(endpoints_);

        std::string host = name + ':' + std::to_string(port);
        ws.handshake(host, "/");

        ws.binary(true);

        std::thread( [&] () {
          boost::asio::spawn(
              io_,
              std::bind(boost::beast::bind_front_handler(
                            &async_thread_websocket_transport::receive, this),
                        std::placeholders::_1));
        }
      );

        io_.run();

        //std::thread t(&async_thread_websocket_transport::receive, this);
        //t.detach();


    }

    std::future<std::vector<unsigned char>> send(unsigned fn_idx,
                                                 std::vector<unsigned char> const& params)
    {
        message ms{ticket_num_++, fn_idx, cista::offset::vector<unsigned char>(params.begin(), params.end())};

        std::promise<std::vector<unsigned char>> promise;
        auto future = promise.get_future();
        if(!tickets_.emplace(ms.ticket_, std::move(promise)).second)
          std::cerr << "could not insert promise!" << std::endl;

        //tickets_.emplace(ms.ticket_, std::promise<std::vector<unsigned char>>());


        //boost::system::error_code error_code;
        ws.write(boost::asio::buffer(cista::serialize(ms)));
        /*
        ws.async_write(boost::asio::buffer(cista::serialize(ms)),[&](boost::system::error_code const& ec, size_t len){
          std::cout << "written!\n";
        });
         */
        std::cout << "write\n";
        return future;
    }

  [[noreturn]] void receive(boost::asio::yield_context y) {
    //boost::asio::yield_context y(std::placeholders::_1);
    std::cout << "started reading!\n";
    while(true) {
      beast::flat_buffer buffer;
      buffer.reserve(1024);

      boost::system::error_code error_code;
      //ws.read(buffer, error_code);
      ws.async_read(buffer, y[error_code]);
      if (error_code != boost::system::errc::errc_t::success) {
        std::cerr << "read nothing! error: " << error_code.message()
                  << std::endl;
        continue;
      }
      std::cout << "corout read after!\n";

      Container con(static_cast<unsigned char*>(buffer.data().data()), buffer.size());
      auto ms = cista::deserialize<message>(con);
      while(true) {
        auto ticket_iter = tickets_.find(ms->ticket_);
        if(ticket_iter != tickets_.end()) {
          ticket_iter->second.set_value(std::vector<unsigned char>(
              ms->payload_.begin(), ms->payload_.end()));
          //tickets_.erase(ms->ticket_);
          //tickets_.erase_meta_only(ticket_iter);
          tickets_.erase(ticket_iter);
          break;
        }
        //std::cout << "failed on rec: " << ms->ticket_ << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
  }

  /*
  [[noreturn]] void receive() {
    //io_.run();
      while(true) {
          beast::flat_buffer buffer;
          buffer.reserve(1024);
          is_reading = true;
          std::cout << "reading loop started!\n";
        /*
        boost::system::error_code error_code;
        ws.read(buffer, error_code);
        if(error_code != boost::system::errc::errc_t::success) {
          std::cerr << "read nothing! error: " << error_code.message() << std::endl;
          continue;
        }
         *
          ws.async_read(buffer, [&](boost::system::error_code const& ec, size_t len)
            {
              std::cout << "reading started!\n";

          Container con(static_cast<unsigned char*>(buffer.data().data()), buffer.size());
          auto ms = cista::deserialize<message>(con);
          while(true) {
            auto ticket_iter = tickets_.find(ms->ticket_);
            if(ticket_iter != tickets_.end()) {
              ticket_iter->second.set_value(std::vector<unsigned char>(
                  ms->payload_.begin(), ms->payload_.end()));
              //tickets_.erase(ms->ticket_);
              //tickets_.erase_meta_only(ticket_iter);
              tickets_.erase(ticket_iter);
              break;
            }
            //std::cout << "failed on rec: " << ms->ticket_ << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
          }
            std::cout << "is finished reading2!\n";
          is_reading = false;
            });
          while (is_reading) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }
          std::cout << "is finished reading!\n";
      }
  }
  */

    ~async_thread_websocket_transport() {
      boost::system::error_code ec;
      ws.async_close(websocket::close_code::normal, [](boost::system::error_code ec){});
      //ws.close(websocket::close_code::normal, ec);
      //if(ec != boost::system::errc::success)
      //  std::cout << "failed to close: " << ec << "\n";
      //std::cout << "closed!\n";
    }

private:
    boost::asio::io_context io_;
    boost::asio::ip::tcp::endpoint endpoints_;

    websocket::stream<tcp::socket> ws;//{io_};

    std::atomic<uint64_t> ticket_num_;
    cista::raw::hash_map<uint64_t, std::promise<std::vector<unsigned char>>> tickets_;
    std::atomic<bool> is_reading;
};

template<typename Interface>
using rpc_async_thread_websocket_client = rpc_async_client<async_thread_websocket_transport, Interface>;