#pragma once

#include <boost/asio/thread_pool.hpp>
#include "crpc/message.h"
#include "crpc/rpc_async_client.h"
#include "crpc/ticket_store.h"
#include "no_network_server.h"

struct no_network_transport {
  explicit no_network_transport(Receiver rec,
                                std::function<void(std::unique_ptr<std::vector<uint8_t>>, std::function<void(std::unique_ptr<std::vector<uint8_t>>)>)> recv)
      : receiver(rec), recv_{recv}{

    work_guard_ = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(boost::asio::make_work_guard(ioc_));
    runner = std::thread([&](){ ioc_.run();});
  }

  void send(std::vector<uint8_t>& ms_buf) {
    recv_(std::move(std::make_unique<std::vector<uint8_t>>(std::move(ms_buf))), lam);
  }

  void receive(std::unique_ptr<std::vector<uint8_t>> response) {
    boost::asio::post(ioc_,
      [this, resp(std::move(response.release()))](){
        receiver.processAnswer(*resp);
        delete resp;
    });
  }

  void stop() {
    work_guard_.reset();
    runner.join();
  }

  private:
    Receiver receiver;
    boost::asio::io_context ioc_;
    std::function<void(std::unique_ptr<std::vector<uint8_t>>, std::function<void(std::unique_ptr<std::vector<uint8_t>>)>)> recv_;
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> work_guard_;
    std::thread runner;
    std::function<void(std::unique_ptr<std::vector<uint8_t>>)> lam = [this](std::unique_ptr<std::vector<uint8_t>> ms){ this->receive(std::move(ms)); };
};

template <typename Interface>
using no_network_client = rpc_async_client<no_network_transport, Interface>;