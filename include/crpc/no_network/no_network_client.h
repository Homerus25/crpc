#pragma once

#include <boost/asio/thread_pool.hpp>
#include "crpc/message.h"
#include "crpc/rpc_async_client.h"
#include "crpc/ticket_store.h"
#include "no_network_server.h"

template<class Serializer>
struct no_network_transport {
private:
  typedef Serializer::SerializedServerMessageContainer SerializedServerMessageContainer;
  typedef Serializer::SerializedClientMessageContainer SerializedClientMessageContainer;

public:
  explicit no_network_transport(Receiver<Serializer> rec,
                                std::function<void(std::unique_ptr<SerializedClientMessageContainer>, std::function<void(std::unique_ptr<SerializedServerMessageContainer>)>)> recv)
      : receiver(rec), recv_{recv}{

    work_guard_ = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(boost::asio::make_work_guard(ioc_));
    runner = std::thread([&](){ ioc_.run();});
  }

  void send(SerializedClientMessageContainer& ms_buf) {
    recv_(std::move(std::make_unique<SerializedClientMessageContainer>(std::move(ms_buf))), lam);
  }

  void receive(std::unique_ptr<SerializedServerMessageContainer> response) {
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
    Receiver<Serializer> receiver;
    boost::asio::io_context ioc_;
    std::function<void(std::unique_ptr<SerializedClientMessageContainer>, std::function<void(std::unique_ptr<SerializedServerMessageContainer>)>)> recv_;
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> work_guard_;
    std::thread runner;
    std::function<void(std::unique_ptr<SerializedServerMessageContainer>)> lam = [this](std::unique_ptr<SerializedServerMessageContainer> ms){ this->receive(std::move(ms)); };
};

template <typename Interface, typename Serializer>
using no_network_client = rpc_async_client<no_network_transport<Serializer>, Interface, Serializer>;